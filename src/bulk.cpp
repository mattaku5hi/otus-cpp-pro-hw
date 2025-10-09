#include <iostream>

#include <string>

#include "async/async.h"


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

    // Single context: read stdin and forward lines as-is
    auto asyncHandle = async::connect(N);

    std::string line;
    while(std::getline(std::cin, line))
    {
        line.push_back('\n');
        async::receive(asyncHandle, line.data(), line.size());
        line.clear();
    }

    async::disconnect(asyncHandle);
    return 0;
}

