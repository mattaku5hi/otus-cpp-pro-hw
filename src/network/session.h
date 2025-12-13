#pragma once

#include <array>
#include <boost/asio.hpp>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "join/database.h"


namespace async 
{

namespace network 
{

namespace asio = boost::asio;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(asio::io_context& io, std::shared_ptr<async::join::Database> db)
        : m_socket(io), m_db(std::move(db)) 
    {}

    asio::ip::tcp::socket& socket()
    {
        return m_socket;
    }

    void start()
    {
        this->doReadLine();
    }

private:
    static void trimCarriageReturn(std::string& s)
    {
        while(!s.empty() && (s.back() == '\r' || s.back() == '\n'))
        {
            s.pop_back();
        }
    }

    static std::vector<std::string> splitBySpaceOnce(const std::string& line)
    {
        std::size_t start = 0;
        std::vector<std::string> out;
        
        for(std::size_t i = 0; i < line.size(); ++i) 
        {
            if(line[i] == ' ') 
            {
                out.emplace_back(line.substr(start, i - start));
                start = i + 1;
                if(out.size() == 2)
                {
                    break;
                }
            }
        }
        out.emplace_back(line.substr(start));
        return out;
    }

    /// @note it’s not recursion in the stack-growth sense. Chain is handler->doReadLine->async_read_until
    /// It’s the standard Boost.Asio “continuation” pattern: the completion handler schedules the next async read and returns.
    void doReadLine()
    {
        auto self = shared_from_this();
        asio::async_read_until(m_socket, m_readBuf, '\n',
            [this, self](const boost::system::error_code& ec, std::size_t) {
                if(!ec)
                {
                    std::istream is(&m_readBuf);
                    std::string line;
                    std::getline(is, line);
                    trimCarriageReturn(line);
                    if(!line.empty())
                    {
                        this->handleCommand(std::move(line));
                    }
                    else
                    {
                        this->doReadLine();
                    }
                }
            });
    }

    void handleCommand(std::string line)
    {
        std::string response;
        
        if(line.rfind("INSERT ", 0) == 0) 
        {
            auto rest = line.substr(7);
            auto parts = splitBySpaceOnce(rest);
            if(parts.size() != 3 || parts[0].size() != 1) 
            {
                response = "< ERR bad command\n";
            } 
            else 
            {
                char tableId = parts[0][0];
                try
                {
                    int id = std::stoi(parts[1]);
                    std::string err;
                    if(m_db->insert(tableId, id, parts[2], err) == true)
                    {
                        response = "< OK\n"; 
                    }
                    else
                    {
                        response = std::string("< ERR ") + err + "\n";
                    }
                } 
                catch(...) 
                {
                    response = "< ERR bad id\n";
                }
            }
        } 
        else if(line.rfind("TRUNCATE ", 0) == 0) 
        {
            auto rest = line.substr(9);
            if(rest.size() != 1) 
            {
                response = "< ERR bad command\n";
            } 
            else 
            {
                std::string err;
                if(m_db->truncate(rest[0], err))
                {
                    response = "< OK\n"; 
                }
                else
                {
                    response = std::string("< ERR ") + err + "\n";
                }
            }
        } 
        else if(line == "INTERSECTION") 
        {
            auto rows = m_db->intersection();
            std::ostringstream os;
            for(auto& t : rows) 
            {
                os << std::get<0>(t) << ',' << std::get<1>(t) << ',' << std::get<2>(t) << '\n';
            }
            os << "< OK\n";
            response = os.str();
        } 
        else if(line == "SYMMETRIC_DIFFERENCE") 
        {
            auto rows = m_db->diffSymmetric();
            std::ostringstream os;
            for(auto& t : rows) 
            {
                os << std::get<0>(t) << ',' << std::get<1>(t) << ',' << std::get<2>(t) << '\n';
            }
            os << "< OK\n";
            response = os.str();
        } 
        else
        {
            response = "< ERR unknown command\n";
        }

        auto self = shared_from_this();
        m_writeBuf = std::move(response);
        asio::async_write(m_socket, asio::buffer(m_writeBuf),
            [this, self](const boost::system::error_code& ec, std::size_t) {
                if(!ec)
                {
                    this->doReadLine();
                }
            }); 
    }

    asio::ip::tcp::socket m_socket;
    std::shared_ptr<async::join::Database> m_db;
    boost::asio::streambuf m_readBuf;
    std::string m_writeBuf;
};

} // namespace network

} // namespace async

