#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "simple_vector.h"


TEST(SimpleVector, EmptyAndSize)
{
    hw3::SimpleVector<int> v(10);
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), 10u);
}

TEST(SimpleVector, PushBackAndIteration)
{
    hw3::SimpleVector<int> v(5);
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    std::vector<int> got;
    for(auto it = v.begin(); it != v.end(); ++it)
    {
        got.push_back(*it);
    }
    ASSERT_EQ(got.size(), 3u);
    EXPECT_EQ(got[0], 1);
    EXPECT_EQ(got[1], 2);
    EXPECT_EQ(got[2], 3);

    // range-based for (non-const)
    got.clear();
    for(int x : v) 
    {
        got.push_back(x);
    }
    ASSERT_EQ(got.size(), 3u);
    EXPECT_EQ(got[0], 1);
    EXPECT_EQ(got[1], 2);
    EXPECT_EQ(got[2], 3);
}

TEST(SimpleVector, ConstIterators)
{
    hw3::SimpleVector<int> v(3);
    v.push_back(10);
    v.push_back(20);

    const auto& cv = v;
    int sum = 0;
    for(auto it = cv.begin(); it != cv.end(); ++it)
    {
        sum += *it;
    }
    EXPECT_EQ(sum, 30);

    // range-based for (const)
    sum = 0;
    for(const int x : cv)
    {
        sum += x;
    }
    EXPECT_EQ(sum, 30);
}

// Custom allocator type (no-op) to verify template parameter compiles

template <class T>
struct DummyAlloc
{
    using value_type = T;
    DummyAlloc() noexcept = default;
    template <class U>
    DummyAlloc(const DummyAlloc<U>&) noexcept 
    {}

    template <class U>
    struct rebind 
    { 
        using other = DummyAlloc<U>; 
    };

    T* allocate(std::size_t n)
    { 
        return std::allocator<T>{}.allocate(n); 
    }
    void deallocate(T* p, std::size_t n) noexcept 
    { 
        std::allocator<T>{}.deallocate(p, n); 
    }

    bool operator==(const DummyAlloc&) const noexcept 
    { 
        return true; 
    }
    bool operator!=(const DummyAlloc&) const noexcept 
    { 
        return false; 
    }
};

TEST(SimpleVector, AcceptsCustomAllocator) 
{
    hw3::SimpleVector<int, DummyAlloc<int>> v(4);
    v.push_back(5);
    v.push_back(6);
    std::vector<int> seq;
    for(auto const& x : v) 
    {
        seq.push_back(x);
    }
    ASSERT_EQ(seq.size(), 2u);
    EXPECT_EQ(seq[0], 5);
    EXPECT_EQ(seq[1], 6);
}

TEST(SimpleVector, OverflowThrows) 
{
    hw3::SimpleVector<int> v(2);
    v.push_back(1);
    v.push_back(2);
    EXPECT_THROW(v.push_back(3), std::bad_alloc);
}

