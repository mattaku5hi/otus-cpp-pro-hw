#pragma once


#include <concepts>
#include <cstdlib>
#include <string>
#include <vector>


namespace csv_parser 
{


template <std::floating_point T>
bool parseRow(const std::string& line, int& label, std::vector<T>& pixels, bool normalize = false)
{
    if(line.empty() == true)
    {
        return false;
    }

    if(pixels.size() != 784)
    {
        pixels.assign(784, static_cast<T>(0));
    }

    const char* p = line.c_str();
    char* pEnd = nullptr;

    long v = std::strtol(p, &pEnd, 10);
    if(pEnd == p)
    {
        return false;
    }
    label = static_cast<int>(v);
    p = (*pEnd == ',') ? pEnd + 1 : pEnd;

    const T kScale = normalize ? static_cast<T>(1.0 / 255.0) : static_cast<T>(1);

    size_t idx = 0;
    while(*p && idx < 784)
    {
        long pv = std::strtol(p, &pEnd, 10);
        if(pEnd == p)
        {
            break;
        }
        pixels[idx++] = static_cast<T>(pv) * kScale;
        p = (*pEnd == ',') ? pEnd + 1 : pEnd;
    }

    return idx == 784;
}


} // namespace csv_parser
