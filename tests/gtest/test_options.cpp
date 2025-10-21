#include <gtest/gtest.h>
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

TEST(OptionsParserTest, Defaults) 
{
    auto opt = parseVec({"bayan"});
    EXPECT_FALSE(opt.showHelp);
    EXPECT_FALSE(opt.showVersion);
    EXPECT_EQ(opt.maxDepth, 1);
    EXPECT_EQ(opt.minSize, 1ULL);
    EXPECT_EQ(opt.blockSize, 4096UL);
    EXPECT_EQ(opt.hash, std::string("md5"));
    EXPECT_TRUE(opt.dirs.empty());
}

TEST(OptionsParserTest, PositionalDirs) 
{
    auto opt = parseVec({"bayan", "/a", "/b"});
    ASSERT_EQ(opt.dirs.size(), 2u);
    EXPECT_EQ(opt.dirs[0], "/a");
    EXPECT_EQ(opt.dirs[1], "/b");
}

TEST(OptionsParserTest, NamePatternsMultiAndCompose) 
{
    auto opt = parseVec({"bayan", "-n", "*.cpp", "*.hpp", "-n", "*.c"});
    ASSERT_EQ(opt.namePatterns.size(), 3u);
    EXPECT_EQ(opt.namePatterns[0], "*.cpp");
    EXPECT_EQ(opt.namePatterns[1], "*.hpp");
    EXPECT_EQ(opt.namePatterns[2], "*.c");
}

TEST(OptionsParserTest, ExcludeDirsCompose) 
{
    auto opt = parseVec({"bayan", "-e", "build", "tmp", "-e", "out"});
    ASSERT_EQ(opt.dirsExclude.size(), 3u);
    EXPECT_EQ(opt.dirsExclude[0], "build");
    EXPECT_EQ(opt.dirsExclude[1], "tmp");
    EXPECT_EQ(opt.dirsExclude[2], "out");
}

TEST(OptionsParserTest, SetScalars) 
{
    auto opt = parseVec({"bayan", "-L", "2", "-m", "1024", "-S", "8192", "-H", "crc32"});
    EXPECT_EQ(opt.maxDepth, 2);
    EXPECT_EQ(opt.minSize, 1024ULL);
    EXPECT_EQ(opt.blockSize, 8192UL);
    EXPECT_EQ(opt.hash, std::string("crc32"));
}

TEST(OptionsParserTest, InvalidHash) 
{
    std::vector<const char*> argv = {"bayan", "-H", "sha1"};
    OptionsParser p;
    EXPECT_THROW(p.parse(static_cast<int>(argv.size()), argv.data()), std::invalid_argument);
}

TEST(OptionsParserTest, HelpFlagImplicit)
{
    auto opt = parseVec({"bayan", "--help"});
    EXPECT_TRUE(opt.showHelp);
    EXPECT_FALSE(opt.showVersion);
    EXPECT_FALSE(opt.helpMessage.empty());
}

TEST(OptionsParserTest, VersionFlagImplicit)
{
    auto opt = parseVec({"bayan", "--version"});
    EXPECT_TRUE(opt.showVersion);
    EXPECT_FALSE(opt.showHelp);
}

TEST(OptionsParserTest, LongOptionsAndDirs)
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
    ASSERT_EQ(opt.dirs.size(), 2u);
    EXPECT_EQ(opt.dirs[0], "/x");
    EXPECT_EQ(opt.dirs[1], "/y");
    ASSERT_EQ(opt.dirsExclude.size(), 1u);
    EXPECT_EQ(opt.dirsExclude[0], "tmp");
    EXPECT_EQ(opt.maxDepth, 3);
    EXPECT_EQ(opt.minSize, 2ULL);
    ASSERT_EQ(opt.namePatterns.size(), 2u);
    EXPECT_EQ(opt.namePatterns[0], "*.png");
    EXPECT_EQ(opt.namePatterns[1], "*.jpg");
    EXPECT_EQ(opt.blockSize, 1024UL);
    EXPECT_EQ(opt.hash, std::string("md5"));
}

TEST(OptionsParserTest, RepeatedDirsShortAndPositional)
{
    auto opt = parseVec({"bayan", "-d", "/a", "/b"});
    ASSERT_EQ(opt.dirs.size(), 2u);
    EXPECT_EQ(opt.dirs[0], "/a");
    EXPECT_EQ(opt.dirs[1], "/b");
}

TEST(OptionsParserTest, InvalidBlockSizeZero)
{
    OptionsParser p;
    std::vector<const char*> argv = {"bayan", "-S", "0"};
    EXPECT_THROW(p.parse(static_cast<int>(argv.size()), argv.data()), std::invalid_argument);
}

TEST(OptionsParserTest, InvalidMinSizeZero)
{
    OptionsParser p;
    std::vector<const char*> argv = {"bayan", "-m", "0"};
    EXPECT_THROW(p.parse(static_cast<int>(argv.size()), argv.data()), std::invalid_argument);
}

TEST(OptionsParserTest, UnknownOptionThrows)
{
    OptionsParser p;
    std::vector<const char*> argv = {"bayan", "--unknown"};
    EXPECT_THROW(p.parse(static_cast<int>(argv.size()), argv.data()), std::exception);
}
