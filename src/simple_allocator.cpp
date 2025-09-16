#include <iostream>
#include <map>
#include <stdexcept>
#include <utility>

#include "arena_allocator.h"
#include "simple_vector.h"


constexpr unsigned long long factorial(unsigned n) noexcept
{
    unsigned long long r = 1;
    for(unsigned i = 2; i <= n; ++i)
    {
        r *= i;
    }

    return r;
}


int main()
{
    // 1) std::map<int,int> with default allocator: fill 0..9 with factorials
    std::map<int, int> mapDefault;
    for(int i = 0; i < 10; ++i)
    {
        mapDefault.emplace(i, factorial(i));
    }

    // 2) std::map<int,int> with new arena allocator limited to ~10 elements
    using Pair = std::pair<const int, int>;
    // allocate the required memory area
    hw3::ArenaAllocator<Pair> arenaPair(10, false); // fixed capacity
    std::map<int, int, std::less<int>, hw3::ArenaAllocator<Pair>> mapArena(std::less<int>(), arenaPair);
    for(int i = 0; i < 10; ++i)
    {
        mapArena.emplace(i, factorial(i));
    }

    // 3) Print all key/value pairs
    std::cout << "=== std::map with default allocator ===\n";
    for(const auto& kv : mapDefault)
    {
        std::cout << kv.first << " " << kv.second << '\n';
    }
    std::cout << "=== std::map with arena allocator ===\n";
    for(const auto& kv : mapArena)
    {
        std::cout << kv.first << " " << kv.second << '\n';
    }

    // 4) Custom container instance for int with default allocator: fill 0..9
    hw3::SimpleVector<int> vecDefault(10);
    for(int i = 0; i < 10; ++i)
    {
        vecDefault.push_back(i);
    }

    // 5) Custom container instance for int with arena allocator limited to 10 elements: fill 0..9
    hw3::ArenaAllocator<int> arenaInt(10, false);
    hw3::SimpleVector<int, hw3::ArenaAllocator<int>> vecArena(10, arenaInt);
    for(int i = 0; i < 10; ++i)
    {
        vecArena.push_back(i);
    }

    // 6) Print all values store in simple containers
    std::cout << "=== SimpleVector with default allocator ===\n";
    for(int x : vecDefault)
    {
        std::cout << x << '\n';
    }
    std::cout << "=== SimpleVector with arena allocator ===\n";
    for(int x : vecArena)
    {
        std::cout << x << '\n';
    }

    return 0;
}
