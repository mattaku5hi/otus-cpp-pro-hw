
#include <boost/program_options.hpp>
#include <sstream>
#include <stdexcept>

#include "options.h"


namespace bayan
{

Options OptionsParser::parse(int argc, const char* const* argv) const
{
    namespace boopo = boost::program_options;  // namespace (not type) alias

    boopo::options_description desc("Bayan options");
    // -> multitoken : allows one occurrence of an option to consume multiple tokens. Example: -n "*.cpp" "*.hpp"
    // -> composing : when the same option appears multiple times, values are appended instead of overwritten. Example: -n "*.cpp" -n "*.hpp"
    // -> is used for method chaining
    desc.add_options()
        ("help", boopo::value<bool>()->implicit_value(true)->default_value(false), "Produce help message")
        ("version", boopo::value<bool>()->implicit_value(true)->default_value(false), "Print version string")
        ("dir,d", boopo::value<std::vector<std::string>>()->multitoken(), "Directories to scan")
        ("exclude-dir,e", boopo::value<std::vector<std::string>>()->multitoken()->composing(), "Directories to exclude")
        ("max-depth,L", boopo::value<int>()->default_value(1), "Maximum recursion depth (1 unlimited, 0 only specified directories)")
        ("min-size,m", boopo::value<unsigned long long>()->default_value(1), "Minimum file size in bytes")
        ("name,n", boopo::value<std::vector<std::string>>()->multitoken()->composing(), "Filename patterns to include (case-insensitive)")
        ("block-size,S", boopo::value<unsigned long>()->default_value(4096), "Read block size in bytes")
        ("hash,H", boopo::value<std::string>()->default_value("md5"), "Hash algorithm (crc32|md5)")
    ;

    boopo::positional_options_description pos;
    // Map all the remaining positional arguments to the "dir" option
    // -1 means "all remaining arguments"
    pos.add("dir", -1);

    boopo::variables_map vm;
    boopo::store(boopo::command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);
    boopo::notify(vm);

    Options opt;
    opt.showHelp = vm["help"].as<bool>();
    opt.showVersion = vm["version"].as<bool>();
    if(vm.count("dir") != 0u)
    {
        opt.dirs = vm["dir"].as<std::vector<std::string>>();
    }
    if(vm.count("exclude-dir") != 0u)
    {
        opt.dirsExclude = vm["exclude-dir"].as<std::vector<std::string>>();
    }
    if(vm.count("name") != 0u)
    {
        opt.namePatterns = vm["name"].as<std::vector<std::string>>();
    }
    opt.maxDepth = vm["max-depth"].as<int>();
    opt.minSize = vm["min-size"].as<unsigned long long>();
    opt.blockSize = vm["block-size"].as<unsigned long>();
    opt.hash = vm["hash"].as<std::string>();

    // Validation
    if(opt.blockSize == 0) 
    {
        throw std::invalid_argument("block-size must be > 0");
    }
    if(opt.minSize == 0)
    {
        throw std::invalid_argument("min-size must be > 0");
    }
    if((opt.hash == "md5" || opt.hash == "crc32") == false)
    {
        throw std::invalid_argument("hash must be one of: crc32, md5");
    }

    std::ostringstream oss;
    oss << desc;
    opt.helpMessage = oss.str();

    return opt;
}

} // namespace bayan

