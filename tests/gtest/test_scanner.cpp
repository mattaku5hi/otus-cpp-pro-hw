
#include <algorithm>
#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <fstream>

#include "options.h"
#include "scanner.h"


namespace fs = boost::filesystem;


namespace 
{

struct TempDir
{
    fs::path dir;
    TempDir() 
    {
        dir = fs::temp_directory_path() / fs::unique_path("bayan-%%%%-%%%%-%%%%");
        fs::create_directories(dir);
    }
    ~TempDir() 
    {
        boost::system::error_code ec;
        fs::remove_all(dir, ec);
    }
};

static void writeFile(const fs::path& p, const std::string& data) 
{
    fs::create_directories(p.parent_path());
    std::ofstream out(p.string(), std::ios::binary);
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
}

static std::string absNorm(const fs::path& p) 
{
    return fs::absolute(p).lexically_normal().string();
}

static bool containsGroup(const std::vector<std::vector<std::string>>& groups,
                          const std::vector<std::string>& expected) 
{
    auto exp = expected;
    std::sort(exp.begin(), exp.end());
    for(const auto& g : groups) 
    {
        if(g == exp)
        {
            return true;
        }
    }
    return false;
}

} // namespace


TEST(ScannerTest, FindsDuplicatesSortedMd5) 
{
    TempDir t;
    const fs::path p1 = t.dir / "a.txt";
    const fs::path p2 = t.dir / "b.txt";
    const fs::path p3 = t.dir / "c.txt";

    std::string data(100, 'A');
    data.replace(50, 1, 1, 'B'); // a slight change mid way
    writeFile(p1, data);
    writeFile(p2, data);
    writeFile(p3, data + "X");

    bayan::Options o;
    o.dirs = { t.dir.string() };
    o.blockSize = 16; // multiple blocks
    o.hash = "md5";
    o.maxDepth = 1; // recursive traversal

    auto groups = bayan::Scanner().findDuplicates(o);

    std::vector<std::string> expected{ absNorm(p1), absNorm(p2) };
    EXPECT_TRUE(containsGroup(groups, expected));
}

TEST(ScannerTest, FindsDuplicatesSortedCrc32) 
{
    TempDir t;
    const fs::path p1 = t.dir / "d1" / "same.bin";
    const fs::path p2 = t.dir / "d2" / "same.bin";

    std::string data;
    for (int i = 0; i < 1024; ++i) data.push_back(static_cast<char>('a' + (i % 26)));
    writeFile(p1, data);
    writeFile(p2, data);

    bayan::Options o;
    o.dirs = { t.dir.string() };
    o.blockSize = 64;
    o.hash = "crc32";
    o.maxDepth = 1;

    auto groups = bayan::Scanner().findDuplicates(o);
    std::vector<std::string> expected{ absNorm(p1), absNorm(p2) };
    EXPECT_TRUE(containsGroup(groups, expected));
}

TEST(ScannerTest, NameFilterAndExclusionAndDepth) 
{
    TempDir t;
    const fs::path pTxt1 = t.dir / "keep" / "a.txt";
    const fs::path pTxt2 = t.dir / "keep" / "b.txt";
    const fs::path pBin  = t.dir / "keep" / "c.bin"; // same content but should be filtered by name
    const fs::path pExcl = t.dir / "exclude" / "b.txt"; // same content but should be excluded
    const fs::path pDeep = t.dir / "level1" / "level2" / "d.txt"; // might be pruned by depth

    std::string data(300, 'Z');
    writeFile(pTxt1, data);
    writeFile(pTxt2, data);
    writeFile(pBin,  data);
    writeFile(pExcl, data);
    writeFile(pDeep, data);

    // Exclude the 'exclude' directory and only include *.txt
    bayan::Options opts;
    opts.dirs = { t.dir.string() };
    opts.dirsExclude = { (t.dir / "exclude").string() };
    opts.namePatterns = { "*.txt" };
    opts.blockSize = 128;
    opts.hash = "md5";

    // Limit to one level below root (so level2 is pruned)
    opts.maxDepth = 2; 

    auto groups = bayan::Scanner().findDuplicates(opts);

    // Expect only the pair inside 'keep' directory, not 'exclude' nor 'level2'
    std::vector<std::string> expected{ absNorm(pTxt1), absNorm(pTxt2) };
    EXPECT_TRUE(containsGroup(groups, expected));

    // Ensure excluded path not present in any group
    const std::string exclAbs = absNorm(pExcl);
    for(const auto& g : groups) 
    {
        for(const auto& s : g) 
        {
            ASSERT_NE(s, exclAbs);
        }
    }
}

