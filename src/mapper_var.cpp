#include <iostream>
#include <string>

#include "csv_utils.h"


constexpr std::string_view DESC = "var";
constexpr char COUNTER = '1';


int main(int argc, char** argv)
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string line;
    while(std::getline(std::cin, line)) 
    {
        std::optional optId = csv_utils::getKey(line); // type deduction
        if(!optId.has_value())
        {
            continue;
        }
        
        std::optional optPrice = csv_utils::getPrice(line);
        if(!optPrice.has_value())
        {
            continue;
        }
        double x2 = optPrice.value() * optPrice.value();
        std::cout << DESC << "\t" << *optId << "\t" << *optPrice << "\t" << x2 << "\t" << COUNTER << "\t\n";
    }

    return EXIT_SUCCESS;
}
