#pragma once


#include <ctime>
#include <string>
#include <vector>


namespace bulkapp 
{

struct Bulk
{
    std::time_t timestamp{};                // time of first command in block (epoch seconds)
    std::vector<std::string> commands;      // commands collected in this block
};


} // namespace bulkapp

