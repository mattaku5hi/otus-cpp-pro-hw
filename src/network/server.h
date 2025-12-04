#pragma once

#include <array>
#include <boost/asio.hpp>
#include <memory>

#include "iasync.h"
#include "session.h"


namespace async
{

namespace network
{

namespace asio = boost::asio;

class Server
    {
    public:
        Server(asio::io_context& io, std::uint16_t port, std::size_t n, const std::shared_ptr<async::IAsyncEngine>& engine)
            : m_io(io), m_acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), m_bulkSize(n), m_engine(engine)
        {
            do_accept();
        }

    private:
        void do_accept()
        {
            auto session = std::make_shared<Session>(m_io, m_bulkSize, m_engine);
            m_acceptor.async_accept(session->socket(),
                [this, session](const boost::system::error_code& ec)
                {
                    if(!ec)
                    {
                        session->start();
                    }
                    do_accept();
                });
        }

        asio::io_context& m_io;
        asio::ip::tcp::acceptor m_acceptor;
        const std::size_t m_bulkSize;
        std::shared_ptr<async::IAsyncEngine> m_engine{};
    };

} // namespace network

} // namespace async

