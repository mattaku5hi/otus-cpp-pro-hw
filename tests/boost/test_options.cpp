
#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE TEST_OPTIONS
#endif


#include <boost/test/included/unit_test.hpp>
#include <stdexcept>

#include "options.h"


using Options = bayan::Options;
using OptionsParser = bayan::OptionsParser;


static Options parseVec(const std::vector<std::string>& args)
{
    std::vector<const char*> argv;
    argv.reserve(args.size());
    for(const auto& s : args)
    {
        argv.push_back(s.c_str());
    }

    OptionsParser p;
    return p.parse(static_cast<int>(argv.size()), argv.data());
}

BOOST_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(defaults) 
{
    auto opt = parseVec({"bayan"});
    BOOST_TEST(opt.showHelp == false);
    BOOST_TEST(opt.showVersion == false);
    BOOST_TEST(opt.maxDepth == 1);
    BOOST_TEST(opt.minSize == 1ULL);
    BOOST_TEST(opt.blockSize == 4096UL);
    BOOST_TEST(opt.hash == std::string("md5"));
}

BOOST_AUTO_TEST_CASE(positional_dirs) 
{
    auto opt = parseVec({"bayan", "/a", "/b"});
    BOOST_REQUIRE(opt.dirs.size() == 2u);
    BOOST_TEST(opt.dirs[0] == "/a");
    BOOST_TEST(opt.dirs[1] == "/b");
}

BOOST_AUTO_TEST_CASE(invalid_hash) 
{
    OptionsParser p;
    std::vector<const char*> argv = {"bayan", "-H", "sha1"};
    BOOST_CHECK_THROW(p.parse(static_cast<int>(argv.size()), argv.data()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(name_patterns_compose)
{
    auto opt = parseVec({"bayan", "-n", "*.cpp", "*.hpp", "-n", "*.c"});
    BOOST_REQUIRE(opt.namePatterns.size() == 3u);
    BOOST_TEST(opt.namePatterns[0] == "*.cpp");
    BOOST_TEST(opt.namePatterns[1] == "*.hpp");
    BOOST_TEST(opt.namePatterns[2] == "*.c");
}

BOOST_AUTO_TEST_CASE(exclude_dirs_compose)
{
    auto opt = parseVec({"bayan", "-e", "build", "tmp", "-e", "out"});
    BOOST_REQUIRE(opt.dirsExclude.size() == 3u);
    BOOST_TEST(opt.dirsExclude[0] == "build");
    BOOST_TEST(opt.dirsExclude[1] == "tmp");
    BOOST_TEST(opt.dirsExclude[2] == "out");
}

BOOST_AUTO_TEST_CASE(set_scalars)
{
    auto opt = parseVec({"bayan", "-L", "2", "-m", "1024", "-S", "8192", "-H", "crc32"});
    BOOST_TEST(opt.maxDepth == 2);
    BOOST_TEST(opt.minSize == 1024ULL);
    BOOST_TEST(opt.blockSize == 8192UL);
    BOOST_TEST(opt.hash == std::string("crc32"));
}

BOOST_AUTO_TEST_CASE(help_flag_implicit)
{
    auto opt = parseVec({"bayan", "--help"});
    BOOST_TEST(opt.showHelp == true);
    BOOST_TEST(opt.showVersion == false);
    BOOST_TEST(!opt.helpMessage.empty());
}

BOOST_AUTO_TEST_CASE(version_flag_implicit)
{
    auto opt = parseVec({"bayan", "--version"});
    BOOST_TEST(opt.showVersion == true);
    BOOST_TEST(opt.showHelp == false);
}

BOOST_AUTO_TEST_CASE(long_options_and_dirs)
{
    auto opt = parseVec({
        "bayan",
        "--dir", "/x",
        "--dir", "/y",
        "--exclude-dir", "tmp",
        "--max-depth", "3",
        "--min-size", "2",
        "--name", "*.png", "*.jpg",
        "--block-size", "1024",
        "--hash", "md5"
    });
    BOOST_REQUIRE(opt.dirs.size() == 2u);
    BOOST_TEST(opt.dirs[0] == "/x");
    BOOST_TEST(opt.dirs[1] == "/y");
    BOOST_REQUIRE(opt.dirsExclude.size() == 1u);
    BOOST_TEST(opt.dirsExclude[0] == "tmp");
    BOOST_TEST(opt.maxDepth == 3);
    BOOST_TEST(opt.minSize == 2ULL);
    BOOST_REQUIRE(opt.namePatterns.size() == 2u);
    BOOST_TEST(opt.namePatterns[0] == "*.png");
    BOOST_TEST(opt.namePatterns[1] == "*.jpg");
    BOOST_TEST(opt.blockSize == 1024UL);
    BOOST_TEST(opt.hash == std::string("md5"));
}

BOOST_AUTO_TEST_CASE(repeated_dirs_short)
{
    auto opt = parseVec({"bayan", "-d", "/a", "/b"});
    BOOST_REQUIRE(opt.dirs.size() == 2u);
    BOOST_TEST(opt.dirs[0] == "/a");
    BOOST_TEST(opt.dirs[1] == "/b");
}

BOOST_AUTO_TEST_CASE(invalid_block_size_zero)
{
    OptionsParser p;
    std::vector<const char*> argv = {"bayan", "-S", "0"};
    BOOST_CHECK_THROW(p.parse(static_cast<int>(argv.size()), argv.data()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(invalid_min_size_zero)
{
    OptionsParser p;
    std::vector<const char*> argv = {"bayan", "-m", "0"};
    BOOST_CHECK_THROW(p.parse(static_cast<int>(argv.size()), argv.data()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(unknown_option_throws)
{
    OptionsParser p;
    std::vector<const char*> argv = {"bayan", "--unknown"};
    BOOST_CHECK_THROW(p.parse(static_cast<int>(argv.size()), argv.data()), std::exception);
}

BOOST_AUTO_TEST_SUITE_END()
