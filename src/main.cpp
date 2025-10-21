
#include <iostream>
#include <string>
#include <vector>

#include "lib_version.h"
#include "options.h"
#include "scanner.h"


int main(int argc, char ** argv)
{
    bayan::OptionsParser parser;
    bayan::Options opts;
	bayan::Scanner scanner;

    try
    {
        opts = parser.parse(argc, const_cast<const char* const*>(argv));
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    if(opts.showHelp == true)
    {
        std::cout << opts.helpMessage << std::endl;
        return 0;
    }

    if(opts.showVersion == true) 
    {
        std::cout << "Version: " << version() << std::endl;
        return 0;
    }

    bool firstGroup{true};
	const auto groups = scanner.findDuplicates(opts);
	for(const auto& g : groups)
    {
        if(g.size() < 2) 
		{
			continue;
		}
        if(firstGroup == false) 
		{
			std::cout << '\n';
		}
        firstGroup = false;
        for(const auto& path : g)
        {
            std::cout << path << '\n';
        }
    }

    return 0;
}
