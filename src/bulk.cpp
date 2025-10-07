#include <iostream>

#include <string>

#include "aggregator.h"
#include "console_sink.h"
#include "file_sink.h"
#include "lib_version.h"
#include "notifier.h"


int main(int argc, char **argv)
{
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <N>" << std::endl;
        return 1;
    }

    std::size_t N = 0;
    try
    {
        N = static_cast<std::size_t>(std::stoul(argv[1]));
    }
    catch(...)
    {
        std::cerr << "Invalid N: must be a positive integer" << std::endl;
        return 1;
    }
    if(N == 0)
    {
        std::cerr << "Invalid N: must be greater than zero" << std::endl;
        return 1;
    }

    bulkapp::Notifier notifier;
    auto console = std::make_shared<bulkapp::ConsoleSink>();
    auto files = std::make_shared<bulkapp::FileSink>("");
    notifier.subscribe(console);
    notifier.subscribe(files);

    bulkapp::Aggregator aggregator{N, notifier};

    std::string line;
    while(std::getline(std::cin, line))
    {
        aggregator.onLine(line);
    }
    aggregator.onEof();

    return 0;
}
