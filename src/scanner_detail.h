#pragma once


#include <boost/crc.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include "options.h"


namespace bayan 
{ 

namespace detail 
{

// Normalize a path to an absolute, lexically-normal form for stable comparisons
inline std::string toAbsoluteNormalized(const boost::filesystem::path& p) 
{
    boost::filesystem::path ap = boost::filesystem::absolute(p);
    return ap.lexically_normal().string();
}

// Returns true if 'path' is inside (or equals) 'prefix', respecting separator boundaries
inline bool isUnder(const std::string& path, const std::string& prefix) 
{
    if(path.size() < prefix.size())
    {
        return false;
    }
    if(path.compare(0, prefix.size(), prefix) != 0)
    {
        return false;
    }
    if(path.size() == prefix.size())
    {
        return true;
    }

    char c = path[prefix.size()];
    return c == '/' || c == '\\';
}

// Convert a simple glob pattern (* and ?) to a full regex (anchored). Case-insensitive is set by caller
inline std::string globToRegex(const std::string& pat) 
{
    std::string rx;
    rx.reserve(pat.size() * 2);
    rx.push_back('^');
    
    for(char ch : pat) 
    {
        switch(ch) 
        {
            case '*': 
            {
                rx += ".*";
                break;
            }
            case '?': 
            {
                rx += '.';
                break;
            }
            case '.': 
            case '+': 
            case '(': 
            case ')': 
            case '^': 
            case '$':
            case '{': 
            case '}': 
            case '|': 
            case '[': 
            case ']': 
            case '\\':
            {
                rx.push_back('\\'); 
                rx.push_back(ch); 
                break;
            }
            default: 
            {
                rx.push_back(ch); 
                break;
            }
        }
    }

    rx.push_back('$');
    return rx;
}

// Lazily computes and caches block hashes for a file to ensure each block is read at most once.
struct BlockHasher 
{
    const Options& opts;
    boost::filesystem::path path;
    uintmax_t fileSize;
    std::vector<std::optional<std::string>> cache; // one per block

    BlockHasher(const Options& o, boost::filesystem::path p, uintmax_t sz)
        : opts(o), path(std::move(p)), fileSize(sz) 
    {
        const uintmax_t blocks = (fileSize + opts.blockSize - 1) / opts.blockSize;
        cache.resize(static_cast<size_t>(blocks));
    }

    size_t blocksCount() const 
    {
        return static_cast<size_t>((fileSize + opts.blockSize - 1) / opts.blockSize);
    }

    // Returns the hash of block 'idx', reading that block only once and padding short reads with zeros.
    const std::string& blockHash(size_t idx) 
    {
        if(cache[idx].has_value())
        {
            return cache[idx].value();
        }

        const uintmax_t offset = static_cast<uintmax_t>(idx) * opts.blockSize;
        std::vector<unsigned char> buf;
        buf.resize(static_cast<size_t>(opts.blockSize), 0);

        std::ifstream in(path.string(), std::ios::binary);
        if(!in) 
        {
            static const std::string empty;
            cache[idx] = empty; // mark to avoid reattempts
            return cache[idx].value();
        }
        if(!in.seekg(static_cast<std::streamoff>(offset), std::ios::beg)) 
        {
            static const std::string empty;
            cache[idx] = empty; // mark to avoid reattempts
            return cache[idx].value();
        }

        in.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(opts.blockSize));
        // if short read, remaining part already zeros

        if(opts.hash == "crc32") 
        {
            boost::crc_32_type crc;
            crc.process_bytes(buf.data(), buf.size());
            uint32_t v = crc.checksum();
            cache[idx] = std::string(reinterpret_cast<const char*>(&v), sizeof(v));
        } 
        else 
        // md5
        { 
            boost::uuids::detail::md5 md5;
            md5.process_bytes(buf.data(), buf.size());
            boost::uuids::detail::md5::digest_type digest;
            md5.get_digest(digest);
            cache[idx] = std::string(reinterpret_cast<const char*>(&digest), sizeof(digest));
        }
        return cache[idx].value();
    }
};

} // namespace detail

} // namespace bayan

