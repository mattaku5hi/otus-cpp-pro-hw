#include <array>
#include <boost/asio.hpp>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "network/server.h"


std::optional<std::uint16_t> parseOptions(int argc, char **argv)
{
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return std::nullopt;
    }

    std::uint16_t portNumber{0};
    try
    {
        portNumber = static_cast<std::uint16_t>(std::stoul(argv[1]));
    }
    catch(...)
    {
        std::cerr << "Invalid port number: must be a positive integer" << std::endl;
        return std::nullopt;
    }
    if(portNumber == 0x01)
    {
        std::cerr << "Invalid port number: the gateway one is reserved" << std::endl;
        return std::nullopt;
    }
    else if(portNumber == 0xffff)
    {
        std::cerr << "Invalid port number: the broadcast one is reserved" << std::endl;
        return std::nullopt;
    }

    return portNumber;
}


int main(int argc, char **argv)
{
    auto opts = parseOptions(argc, argv);
    if(!opts)
    {
        return EXIT_FAILURE;
    }
    const auto portNumber = *opts;

    try
    {
        boost::asio::io_context io;
        // launch for all the local interfaces (default)
        async::network::Server server(io, portNumber);
        io.run();
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Server error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
