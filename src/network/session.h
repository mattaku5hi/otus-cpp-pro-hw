#pragma once


#include <array>
#include <boost/asio.hpp>
#include <memory>

#include "iasync.h"


namespace async
{

namespace network
{

namespace asio = boost::asio;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(asio::io_context& io, std::size_t n, const std::shared_ptr<async::IAsyncEngine>& engine)
        : m_socket(io), m_bulkSize(n), m_engine(engine) {}

    asio::ip::tcp::socket& socket() 
    { 
        return m_socket; 
    }

    void start()
    {
        m_handle = m_engine->connect(m_bulkSize);
        this->doRead();
    }

private:
    void doRead()
    {
        auto self = shared_from_this();
        m_socket.async_read_some(
            asio::buffer(m_buffer),
            [this, self](const boost::system::error_code& ec, std::size_t bytes)
            {
                if(!ec && bytes > 0)
                {
                    m_engine->receive(m_handle, m_buffer.data(), bytes);
                    this->doRead();
                }
                else
                {
                    m_engine->disconnect(m_handle);
                }
            });
    }

    asio::ip::tcp::socket m_socket;
    const std::size_t m_bulkSize;
    std::shared_ptr<async::IAsyncEngine> m_engine{};
    async::handle_t m_handle{};
    std::array<char, 4096> m_buffer{};
};

} // namespace network

} // namespace async
