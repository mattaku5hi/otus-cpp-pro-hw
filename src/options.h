#pragma once

#include <string>
#include <vector>


namespace bayan
{

struct Options 
{
    std::vector<std::string> dirs;
    std::vector<std::string> dirsExclude;
    int maxDepth{1};
    unsigned long long minSize{1};
    std::vector<std::string> namePatterns;
    unsigned long blockSize{4096};
    std::string hash{ "md5" };
    bool showHelp{false};
    bool showVersion{false};
    std::string helpMessage;
};

class OptionsParser 
{

public:
    Options parse(int argc, const char* const* argv) const;

};

}

