#include <array>
#include <boost/asio.hpp>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tuple>

#include "async/async.h"
#include "async/dispatcher.h"
#include "network/server.h"


std::optional<std::tuple<std::uint16_t, std::size_t>> parseOptions(int argc, char **argv)
{
    if(argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <bulk_size>" << std::endl;
        return std::nullopt;
    }

    std::uint16_t portNumber{0};
    std::size_t bulkSize{0};
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
    try
    {
        bulkSize = static_cast<std::size_t>(std::stoul(argv[2]));
    }
    catch(...)
    {
        std::cerr << "Invalid N: must be a positive integer" << std::endl;
        return std::nullopt;
    }
    if(bulkSize == 0)
    {
        std::cerr << "Invalid N: must be greater than zero" << std::endl;
        return std::nullopt;
    }

    return std::make_tuple(portNumber, bulkSize);
}


int main(int argc, char **argv)
{
    auto opts = parseOptions(argc, argv);
    if(!opts)
    {
        return EXIT_FAILURE;
    }
    const auto [portNumber, bulkSize] = *opts;

    try
    {
        auto dispatcher = std::make_shared<async::Dispatcher>();
        auto engine = async::createEngine(dispatcher);
        boost::asio::io_context io;
        async::network::Server server(io, portNumber, bulkSize, engine);
        io.run();
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Server error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


