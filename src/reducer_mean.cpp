#include <iostream>
#include <sstream>
#include <string>


constexpr std::string_view DESC = "mean";


int main(int argc, char** argv) 
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string line;
    long long count = 0;
    long double sum = 0.0L;

    while(std::getline(std::cin, line)) 
    {
        // Expect: mean \t key \t price \t 1 \t
        std::istringstream iss(line);
        std::string descStr, idStr, priceStr, cntStr;
        if(!std::getline(iss, descStr, '\t') || descStr != DESC)
        {
            std::cerr << "Invalid line format\n";
            return EXIT_FAILURE;
        }
        if(!std::getline(iss, idStr, '\t'))
        {
            std::cerr << "Empty id csv field\n";
            continue;
        }
        if(!std::getline(iss, priceStr, '\t'))
        {
            std::cerr << "Empty price csv field\n";
            continue;
        }
        if(!std::getline(iss, cntStr, '\t'))
        {
            std::cerr << "Empty counter csv field\n";
            continue;
        }
        try 
        {
            long double price = std::stold(priceStr);
            if(price < 0)
            {
                std::cerr << "Invalid price value: " << price << "\n";
                continue;
            }
            long long c = std::stoll(cntStr);
            if(c <= 0)
            {
                std::cerr << "Invalid count value: " << c << "\n";
                continue;
            }
            sum += price;
            count += c;
        } 
        catch(...) 
        {
            continue;
        }
    }

    if(count > 0) 
    {
        long double mean = sum / static_cast<long double>(count);
        std::cout << static_cast<double>(mean) << "\n";
    }
    return 0;
}
