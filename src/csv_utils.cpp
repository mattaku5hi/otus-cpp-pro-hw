#include <charconv>
#include <iostream>
#include <string_view>

#include "csv_utils.h"


namespace csv_utils
{

std::string_view getField(const std::string_view line, size_t target_idx) 
{
    size_t start = 0;
    size_t idx = 0;
    bool isInQuotes = false;
    
    for(size_t i = 0; i <= line.size(); ++i)
    {
        char c = (i < line.size() ? line[i] : ',');
        if(c == '"') 
        {
            isInQuotes = !isInQuotes;
        } 
        else if(c == ',' && !isInQuotes) 
        {
            if(idx == target_idx)
            {
                return line.substr(start, i - start);
            }
            ++idx;
            start = i + 1;
        }
    }

    return {};
}

std::optional<std::string_view> getKey(const std::string_view line) noexcept
{
    std::string_view idStr = getField(line, 0); // 1th column (id)
    if(idStr.empty())
    {
        return std::nullopt; 
    }

    return idStr;
}


std::optional<double> getPrice(const std::string_view line) noexcept
{
    std::string_view priceStr = getField(line, 9); // 10th column
    if(priceStr.empty())
    {
        return std::nullopt; 
    }
    try
    {
        double price;
        std::from_chars_result result = std::from_chars(priceStr.data(), priceStr.data() + priceStr.size(), price, std::chars_format::general);
        if(result.ec != std::errc() || result.ptr != priceStr.data() + priceStr.size())
        {
            return std::nullopt;
        }
        return price;
    } 
    catch(...) 
    {
        return std::nullopt;
    }
}

} // namespace csv_utils
