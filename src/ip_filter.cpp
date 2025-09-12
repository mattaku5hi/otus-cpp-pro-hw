#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "ip_tools.h"


int main(int, char const*[]) 
{
    try 
    {
        // Read IPv4 addresses from stdin (only the first tab-separated field)
        auto ipPool = iptools::ReadInputIPs(std::cin);

        // Reverse lexicographical sort by bytes (numeric)
        iptools::SortIPsDesc(ipPool);

        // 1) Print full list sorted in reverse order
        iptools::PrintIps(ipPool, std::cout);

        // 2) Filter by first byte == 1
        const auto f1 = iptools::FilterSecondStep(ipPool, 1);
        iptools::PrintIps(f1, std::cout);

        // 3) Filter by first byte == 46 and second byte == 70
        const auto f46_70 = iptools::FilterThirdStep(ipPool, 46, 70);
        iptools::PrintIps(f46_70, std::cout);

        // 4) Filter by any byte == 46
        const auto any46 = iptools::FilterFourthStep(ipPool, 46);
        iptools::PrintIps(any46, std::cout);
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return 666;
    }

    return 0;
}