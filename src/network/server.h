#pragma once

#include <array>
#include <boost/asio.hpp>
#include <memory>

#include "session.h"


namespace async
{

namespace network
{

class Server
{
public:
    Server(asio::io_context& io, std::uint16_t port)
        : m_io(io), m_acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
        this->doAccept();
    }

private:
    void doAccept()
    {
        auto session = std::make_shared<Session>(m_io);
        m_acceptor.async_accept(session->socket(),
            [this, session](const boost::system::error_code& ec) {
                if(!ec)
                {
                    session->start();
                }
                this->doAccept();
            });
    }

    asio::io_context& m_io;
    asio::ip::tcp::acceptor m_acceptor;
};

} // namespace network

} // namespace async

