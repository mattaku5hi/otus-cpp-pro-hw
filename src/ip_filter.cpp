#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "ip_tools.h"


int main(int argc, char const* argv[]) 
{
    // Basic check
    if(argc > 1)
    {
        std::cerr << "Usage: " << argv[0] << "\n";
        return 555;
    }

    try 
    {
        // Read IPv4 addresses from stdin (only the first tab-separated field)
        auto ipPool = ip_filter::ReadInputIps(std::cin);

        // Reverse lexicographical sort by bytes (numeric)
        ip_filter::SortIpsDesc(ipPool);

        // 1) Print full list sorted in reverse order
        ip_filter::PrintIp(ipPool, std::cout);

        // 2) Filter by first byte == 1
        const auto f1 = ip_filter::FilterSecondStep(ipPool, 1);
        ip_filter::PrintIp(f1, std::cout);

        // 3) Filter by first byte == 46 and second byte == 70
        const auto f46_70 = ip_filter::FilterThirdStep(ipPool, 46, 70);
        ip_filter::PrintIp(f46_70, std::cout);

        // 4) Filter by any byte == 46
        const auto any46 = ip_filter::FilterFourthStep(ipPool, 46);
        ip_filter::PrintIp(any46, std::cout);
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return 666;
    }

    return 0;
}