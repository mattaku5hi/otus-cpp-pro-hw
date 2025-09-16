#include <gtest/gtest.h>
#include <map>
#include <stdexcept>
#include <utility>

#include "arena_allocator.h"


TEST(ArenaAllocator, FixedCapacity_AllocateWithinCapacityThenThrow)
{
    hw3::ArenaAllocator<int> alloc(10, false); // fixed capacity: 10
    int* p1 = nullptr; 
    int* p2 = nullptr;
    ASSERT_NO_THROW({ p1 = alloc.allocate(6); });
    ASSERT_NE(p1, nullptr);
    ASSERT_NO_THROW({ p2 = alloc.allocate(4); });
    ASSERT_NE(p2, nullptr);
    // Next should throw
    EXPECT_THROW({ (static_cast<void>(alloc.allocate(1))); }, std::bad_alloc);
}

TEST(ArenaAllocator, AutoResize_GrowsToFitLargeAndFurther)
{
    hw3::ArenaAllocator<int> alloc(10, true); // resizable
    int* p1 = nullptr; int* p2 = nullptr;
    EXPECT_NO_THROW({ p1 = alloc.allocate(12); });
    EXPECT_NO_THROW({ p2 = alloc.allocate(30); });
    EXPECT_NE(p1, nullptr);
    EXPECT_NE(p2, nullptr);
}

struct Cntr
{
    static int ctor;
    static int dtor;
    int v{0};
    explicit Cntr(int x) : v(x) 
    { 
        ++ctor; 
    }
    ~Cntr() 
    { 
        ++dtor; 
    }
};
int Cntr::ctor = 0;
int Cntr::dtor = 0;

TEST(ArenaAllocator, ConstructDestroy_CallsCtorsAndDtors)
{
    Cntr::ctor = 0;
    Cntr::dtor = 0;
    hw3::ArenaAllocator<Cntr> alloc(4, false);
    Cntr* pMem = alloc.allocate(3);
    using Traits = std::allocator_traits<hw3::ArenaAllocator<Cntr>>;
    Traits::construct(alloc, pMem + 0, 1);
    Traits::construct(alloc, pMem + 1, 2);
    Traits::construct(alloc, pMem + 2, 3);
    EXPECT_EQ(Cntr::ctor, 3);
    Traits::destroy(alloc, pMem + 0);
    Traits::destroy(alloc, pMem + 1);
    Traits::destroy(alloc, pMem + 2);
    EXPECT_EQ(Cntr::dtor, 3);
}

TEST(ArenaAllocator, StdMap_AutoResize_NoThrowForMany)
{
    using Pair = std::pair<const int, int>;
    hw3::ArenaAllocator<Pair> a(8, true); // resizable map allocator
    std::map<int, int, std::less<int>, hw3::ArenaAllocator<Pair>> m(std::less<int>(), a);
    for(int i = 0; i < 100; ++i)
    {
        EXPECT_NO_THROW({ m.emplace(i, i*i); }) << "Failed at i=" << i;
    }
    EXPECT_EQ(m.size(), 100u);
}

TEST(ArenaAllocator, StdMap_FixedCapacity_ThrowsWhenExceeded)
{
    using Pair = std::pair<const int, int>;
    // Provide a small margin for potential internal nodes
    hw3::ArenaAllocator<Pair> a(12, false);
    std::map<int, int, std::less<int>, hw3::ArenaAllocator<Pair>> m(std::less<int>(), a);

    // First 10 inserts should succeed
    for(int i = 0; i < 10; ++i) 
    {
        EXPECT_NO_THROW({ m.emplace(i, i); }) << "Failed at i=" << i;
    }

    // Inserting significantly more must throw at some point
    bool threw = false;
    try 
    {
        for(int i = 10; i < 100; ++i)
        {
            m.emplace(i, i);
        }
    } 
    catch(const std::bad_alloc&)
    {
        threw = true;
    }
    EXPECT_TRUE(threw);
}


