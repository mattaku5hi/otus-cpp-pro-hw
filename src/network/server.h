#pragma once

#include <array>
#include <boost/asio.hpp>
#include <memory>

#include "session.h"
#include "join/database.h"


namespace async
{

namespace network
{
 namespace asio = boost::asio;

class Server
{
public:
    Server(asio::io_context& io, std::uint16_t port)
        : m_io(io), m_acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
        m_db = std::make_shared<async::join::Database>();
        this->doAccept();
    }

private:
    void doAccept()
    {
        auto session = std::make_shared<Session>(m_io, m_db);
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
    std::shared_ptr<async::join::Database> m_db;
};

} // namespace network

} // namespace async

