
#include <boost/crc.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/uuid/detail/md5.hpp>

#include <algorithm>
#include <fstream>
#include <map>
#include <optional>
#include <unordered_map>
#include "scanner.h"
#include "scanner_detail.h"


namespace boofs = boost::filesystem;

namespace bayan 
{

using namespace bayan::detail;


std::vector<std::vector<std::string>> Scanner::findDuplicates(const Options& opts) const 
{
    // 1) Prepare absolute excluded directories for fast prefix checks
    std::vector<std::string> excludeAbs;
    excludeAbs.reserve(opts.dirsExclude.size());
    for(const auto& d : opts.dirsExclude)
    {
        excludeAbs.push_back(toAbsoluteNormalized(d));
    }

    // 2) Compile name filters (glob -> regex, case-insensitive)
    // Booom! vector of boost::regex !!!
    // It compiles the pattern into an internal representation (AST/NFA) and uses a backtracking engine (Perl-like).
    // Supports traits/locales and different grammars (perl, extended, etc.). We use perl + icase.
    std::vector<boost::regex> nameRegexes;
    nameRegexes.reserve(opts.namePatterns.size());
    for(const auto& pat : opts.namePatterns)
    {
        nameRegexes.emplace_back(globToRegex(pat), boost::regex::perl | boost::regex::icase);
    }

    // 3) Collect candidate files grouped by size to avoid needless reads
    std::unordered_map<uintmax_t, std::vector<boofs::path>> sizeGroups;
    // File filter: regular file, min size, matches any name pattern if provided
    auto includeFile = [&](const boofs::directory_entry& ent) -> bool {
        if(boofs::is_regular_file(ent.path()) == false)
        {
            return false;
        }

        boost::system::error_code ec;
        auto sz = boofs::file_size(ent.path(), ec);
        if(ec)
        {
            return false;
        }
        if(sz < opts.minSize)
        {
            return false;
        }
        // We have name filter defined so check them 
        if(nameRegexes.empty() == false)
        {
            const std::string name = ent.path().filename().string();
            bool any = false;
            for(const auto& rx : nameRegexes)
            {
                if(boost::regex_match(name, rx))
                {
                    any = true;
                    break;
                }
            }
            if(any == false)
            {
                return false;
            }
        }
        return true;
    };

    // Directory filter: skip if under any excluded root
    auto shouldExcludeDir = [&](const boofs::path& p) -> bool
    {
        const std::string abs = toAbsoluteNormalized(p);
        for(const auto& ex : excludeAbs)
        {
            if(isUnder(abs, ex) == true)
            {
                return true;
            }
        }
        return false;
    };

    // Walk a root directory according to maxDepth, honoring exclusions, and bucket files by size
    auto addDir = [&](const boofs::path& root) -> void
    {
        if(boofs::exists(root) == false)
        {
            return;
        }
        if(shouldExcludeDir(root))
        {
            return;
        }
        const bool noRecursion = (opts.maxDepth == 0);
        if(noRecursion == true)
        {
            // Boooom! directory iterator in Boost, how about that? :)
            for(boofs::directory_iterator it(root), end; it != end; ++it)
            {
                if(includeFile(*it))
                {
                    boost::system::error_code ec;
                    auto sz = boofs::file_size(it->path(), ec);
                    if(ec.failed() == false)
                    {
                        sizeGroups[sz].push_back(boofs::absolute(it->path()));
                    }
                }
            }
        } 
        else 
        {
            // ... And the recursive one here!!!
            boofs::recursive_directory_iterator it(root), end;
            while(it != end) 
            {
                const boofs::path p = it->path();
                if(boofs::is_directory(p) == true) 
                {
                    if(shouldExcludeDir(p) == true)
                    {
                        it.disable_recursion_pending(); 
                        ++it; 
                        continue; 
                    }
                    if(opts.maxDepth > 1) 
                    {
                        // limit recursion: depth() counts how many directories below root we are
                        if(it.depth() >= (opts.maxDepth - 1)) 
                        { 
                            it.disable_recursion_pending(); 
                        }
                    }
                }
                else if(includeFile(*it) == true)
                {
                    boost::system::error_code ec;
                    auto sz = boofs::file_size(p, ec);
                    if(ec.failed() == false)
                    {
                        sizeGroups[sz].push_back(boofs::absolute(p));
                    }
                }
                ++it;
            }
        }
    };

    if(opts.dirs.empty() == true) 
    {
        addDir(boofs::path("."));
    } 
    else 
    {
        for(const auto& d : opts.dirs)
        {
            addDir(d);
        }
    }

    // 4) Compare within size groups using progressive block hashing (read-on-demand)
    std::vector<std::vector<std::string>> result;

    for(auto& kv : sizeGroups)
    {
        auto& files = kv.second;
        // if nothing to compare
        if(files.size() < 2)
        {
            continue;
        }

        // Build hasher list
        std::vector<BlockHasher> hashers;
        hashers.reserve(files.size());
        for(const auto& p : files)
        {
            boost::system::error_code ec;
            auto sz = boofs::file_size(p, ec);
            if(ec.failed() == true)
            {
                continue;
            }
            hashers.emplace_back(opts, p, sz);
        }
        // if nothing to compare
        if(hashers.size() < 2)
        {
            continue;
        }

        // Start with a single candidate group of indices (all files of this size)
        std::vector<std::vector<size_t>> groups;
        groups.push_back({});
        groups.back().reserve(hashers.size());
        for(size_t i = 0; i < hashers.size(); ++i)
        {
            groups.back().push_back(i);
        }

        // Iterate blocks progressively; only tied files are read at each block
        size_t maxBlocks{0};
        for(const auto& h : hashers)
        {
            if(h.blocksCount() > maxBlocks)
            {
                maxBlocks = h.blocksCount();
            }
        }

        for(size_t b = 0; b < maxBlocks; ++b)
        {
            std::vector<std::vector<size_t>> nextGroups;
            nextGroups.reserve(groups.size());
            for(auto& g : groups)
            {
                if(g.size() < 2)
                {
                    continue;
                }
                // Partition by block hash at b (sizes are equal across this bucket)
                std::unordered_map<std::string, std::vector<size_t>> part;
                part.reserve(g.size());
                for(size_t idx : g) 
                {
                    const std::string& h = hashers[idx].blockHash(b);
                    part[h].push_back(idx);
                }
                for(auto& pkv : part)
                {
                    if(pkv.second.size() > 1)
                    {
                        nextGroups.push_back(std::move(pkv.second));
                    }
                }
            }
            if(nextGroups.empty() == true)
            {
                groups.clear();
                break;
            }
            groups.swap(nextGroups);
        }

        // Emit duplicate groups (size > 1)
        for(const auto& g : groups)
        {
            if(g.size() < 2)
            {
                continue;
            }
            std::vector<std::string> out;
            out.reserve(g.size());
            for(size_t idx : g)
            {
                out.push_back(hashers[idx].path.string());
            }
            result.push_back(std::move(out));
        }
    }

    // Deterministic output: sort paths within groups, then sort groups lexicographically
    for(auto& g : result) 
    {
        std::sort(g.begin(), g.end());
    }
    std::sort(result.begin(), result.end());

    return result;
}

} // namespace bayan

