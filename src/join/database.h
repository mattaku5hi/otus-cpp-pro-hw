#pragma once

#include <map>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

namespace async 
{

namespace join 
{


class Database 
{
public:
    // tableId: 'A' or 'B'
    bool insert(char tableId, int id, const std::string& name, std::string& err) 
    {
        auto* tbl = tableFor(tableId, err);
        if(!tbl)
        {
            return false;
        }
        
        bool isInserted = false;
        {
            std::scoped_lock lk(m_mtx);
            isInserted = tbl->emplace(id, name).second;
        }
        if(!isInserted) 
        {
            err = std::string("duplicate ") + std::to_string(id);
            return false;
        }

        return true;
    }

    bool truncate(char tableId, std::string& err) 
    {
        auto* tbl = tableFor(tableId, err);
        if(!tbl) 
        {
            return false;
        }

        {
            std::scoped_lock lk(m_mtx);
            tbl->clear();
        }

        return true;
    }

    // Returns sorted by id
    std::vector<std::tuple<int, std::string, std::string>> intersection() const 
    {
        std::vector<std::tuple<int, std::string, std::string>> out;
        std::scoped_lock lk(m_mtx);
        auto itA = m_A.begin();
        auto itB = m_B.begin();
        while(itA != m_A.end() && itB != m_B.end()) 
        {
            if(itA->first == itB->first) 
            {
                out.emplace_back(itA->first, itA->second, itB->second);
                ++itA; ++itB;
            } 
            else if(itA->first < itB->first) 
            {
                ++itA;
            } 
            else
            {
                ++itB;
            }
        }
        return out;
    }

    // Returns sorted by id
    std::vector<std::tuple<int, std::string, std::string>> diffSymmetric() const 
    {
        std::vector<std::tuple<int, std::string, std::string>> out;
        std::scoped_lock lk(m_mtx);
        auto itA = m_A.begin();
        auto itB = m_B.begin();
        while(itA != m_A.end() || itB != m_B.end()) 
        {
            if(itB == m_B.end() || (itA != m_A.end() && itA->first < itB->first)) 
            {
                out.emplace_back(itA->first, itA->second, std::string());
                ++itA;
            } 
            else if(itA == m_A.end() || itB->first < itA->first) 
            {
                out.emplace_back(itB->first, std::string(), itB->second);
                ++itB;
            } 
            else // equal -> skip (in intersection) 
            {
                ++itA; ++itB;
            }
        }
        return out;
    }

private:
    std::map<int, std::string>* tableFor(char tableId, std::string& err) 
    {
        if(tableId == 'A')
        {
            return &m_A;
        }
        if(tableId == 'B')
        {
            return &m_B;
        }
        err = "unknown table";
        return nullptr;
    }

    const std::map<int, std::string>* tableFor(char tableId, std::string& err) const 
    {
        if(tableId == 'A')
        {
            return &m_A;
        }
        if(tableId == 'B')
        {
            return &m_B;
        }
        
        err = "unknown table";
        return nullptr;
    }

    mutable std::mutex m_mtx;
    std::map<int, std::string> m_A;
    std::map<int, std::string> m_B;
};

} // namespace join
} // namespace async
