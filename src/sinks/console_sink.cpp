
#include <iostream>

#include "bulk.h"
#include "console_sink.h"


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

void ConsoleSink::onBulk(const Bulk& b)
{
    std::cout << lineFormat(b) << std::endl;
}


} // namespace bulkapp

