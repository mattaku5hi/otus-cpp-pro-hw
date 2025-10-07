
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
    name << "bulk" << b.timestamp << ".log";

    std::ofstream ofs(name.str());
    if(ofs.is_open() == false)
    {
        std::cerr << "Failed to open file: " << name.str() << std::endl;
        return;
    }

    ofs << lineFormat(b) << '\n';
}

} // namespace bulkapp

