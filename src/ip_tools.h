#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


namespace ip_filter
{

using IPv4 = std::array<int, 4>; // alias

inline std::vector<std::string> split(const std::string& str, char delimiter) 
{
    std::vector<std::string> result;
    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(delimiter);
    while(stop != std::string::npos) 
    {
        result.push_back(str.substr(start, stop - start));
        start = stop + 1;
        stop = str.find_first_of(delimiter, start);
    }
    result.push_back(str.substr(start));
    
    return result;
}

// C++20 provides std::in_range; for C++17 we define a simple helper
template <class T, class U>
constexpr bool isInRange(U value)
{
    return value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max();
}

inline IPv4 ParseIpv4(const std::string& text) 
{
    const auto parts = split(text, '.');
    if(parts.size() != 4) 
    {
        throw std::runtime_error("Invalid IPv4: wrong parts count: " + text);
    }
    IPv4 ip{};
    for(size_t i = 0; i < 4; ++i) 
    {
        const auto& p = parts[i];
        if(p.empty() == true) 
        {
            throw std::runtime_error("Invalid IPv4: empty octet: " + text);
        }
        int val = 0;
        // no leading/trailing spaces expected in correct input, but be defensive:
        // stoi throws on invalid; we additionally check range 0..255
        size_t pos = 0;
        try
        {
            val = std::stoi(p, &pos);
        } 
        catch(...)
        {
            throw std::runtime_error("Invalid IPv4: non-integer octet: " + text);
        }
        if(pos != p.size() || isInRange<uint8_t>(val) == false)
        {
            throw std::runtime_error("Invalid IPv4: out of range octet: " + text);
        }
        ip[i] = val;
    }
    return ip;
}

inline std::vector<IPv4> ReadInputIps(std::istream& in) 
{
    std::vector<IPv4> ips;
    std::string line;
    while(std::getline(in, line)) 
    {
        if(line.empty() == true)
        {
            continue;
        }
        const auto tab = line.find('\t');
        const std::string addr = (tab == std::string::npos) ? line : line.substr(0, tab);
        if(addr.empty() == true)
        {
            continue;
        }
        ips.emplace_back(ParseIpv4(addr));
    }
    return ips;
}

inline void SortIpsDesc(std::vector<IPv4>& ips) 
{
    std::sort(ips.begin(), ips.end(), std::greater<IPv4>());
}

// Could be hidden as private
inline void PrintIpImpl(const IPv4& ip, std::ostream& out) 
{
    out << ip[0] << '.' << ip[1] << '.' << ip[2] << '.' << ip[3] << '\n';
}

// delegate
inline void PrintIp(const IPv4& ip, std::ostream& out)
{
    PrintIpImpl(ip, out);
}

template <typename Container>
inline void PrintIp(const Container& ips, std::ostream& out)
{
    using Elem = std::remove_cv_t<typename Container::value_type>;
    static_assert(std::is_same<Elem, IPv4>::value,
                  "PrintIp(Container): Container::value_type must be iptools::IPv4");
    for (const auto& ip : ips)
    {
        PrintIpImpl(ip, out);
    }
}

inline std::vector<IPv4> FilterSecondStep(const std::vector<IPv4>& ips, int value) 
{
    std::vector<IPv4> out;
    std::copy_if(ips.begin(), ips.end(), std::back_inserter(out),
                 [value](const IPv4& ip) { 
                    return ip[0] == value;
                 });
    return out;
}

inline std::vector<IPv4> FilterThirdStep(const std::vector<IPv4>& ips, int value1, int value2) 
{
    std::vector<IPv4> out;
    std::copy_if(ips.begin(), ips.end(), std::back_inserter(out),
                 [value1, value2](const IPv4& ip) { 
                    return ip[0] == value1 && ip[1] == value2;
                 });
    return out;
}

inline std::vector<IPv4> FilterFourthStep(const std::vector<IPv4>& ips, int value) 
{
    std::vector<IPv4> out;
    std::copy_if(ips.begin(), ips.end(), std::back_inserter(out),
                 [value](const IPv4& ip) {
                     return ip[0] == value || ip[1] == value || ip[2] == value || ip[3] == value;
                 });
    return out;
}

} // namespace ip_filter