
#include <fstream>
#include <iostream>
#include <sstream>

#include "bulk.h"
#include "file_sink.h"


namespace bulkapp 
{

static std::string lineFormat(const Bulk& b)
{
    std::string out = "bulk: ";
    for(std::size_t i = 0; i < b.commands.size(); ++i)
    {
        out += b.commands[i];
        if(i + 1 < b.commands.size())
        {
            out += ", ";
        }
    }
    return out;
}

void FileSink::onBulk(const Bulk& b)
{
    std::ostringstream name;
    if(m_dir.empty() == false)
    {
        if(m_dir.back() == '/' || m_dir.back() == '\\')
        {
            name << m_dir;
        }
        else
        {
            name << m_dir << '/';
        }
    }
    
    // Build unique filename: bulk<timestamp>_<threadid>_<seq>.log
    unsigned long long local_seq = m_seq.fetch_add(1, std::memory_order_relaxed);
    name << "bulk" << b.timestamp << "_";
    if(m_threadId.empty() == false) 
    {
        name << m_threadId << "_";
    }
    name << local_seq << ".log";

    std::ofstream ofs(name.str());
    if(ofs.is_open() == false)
    {
        std::cerr << "Failed to open file: " << name.str() << std::endl;
        return;
    }

    ofs << lineFormat(b) << '\n';
}

} // namespace bulkapp

