#include <gtest/gtest.h>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "print_ip.h"


using namespace hw3;


// Helper to capture std::cout
class CoutCapture 
{
public:
    CoutCapture() : 
    m_pOriginal(std::cout.rdbuf(m_oss.rdbuf())) 
    {}
    ~CoutCapture() 
    { 
        std::cout.rdbuf(m_pOriginal); 
    }
    std::string str() const 
    { 
        return m_oss.str(); 
    }
private:
    std::streambuf* m_pOriginal;
    std::ostringstream m_oss;
};


// Trait to detect uniform tuples without instantiating print_ip
namespace 
{

template <class T>
struct uniform_tuple : std::false_type {};

template <class... Ts>
struct uniform_tuple<std::tuple<Ts...>>
    : std::bool_constant<(sizeof...(Ts) > 0 && (std::is_same_v<Ts, std::tuple_element_t<0, std::tuple<Ts...>>> && ...))> {};

} // namespace


TEST(PrintIp, IntegralCases) 
{
    {
        CoutCapture cap;
        print_ip(int8_t{-1});
        EXPECT_EQ(cap.str(), std::string("255\n"));
    }
    {
        CoutCapture cap;
        print_ip(int16_t{0});
        EXPECT_EQ(cap.str(), std::string("0.0\n"));
    }
    {
        CoutCapture cap;
        print_ip(int32_t{2130706433});
        EXPECT_EQ(cap.str(), std::string("127.0.0.1\n"));
    }
    {
        CoutCapture cap;
        print_ip(int64_t{8875824491850138409});
        EXPECT_EQ(cap.str(), std::string("123.45.67.89.101.112.131.41\n"));
    }
}

TEST(PrintIp, StringCase) 
{
    {
        const char* pEx = "Whatever you do in life will be insignificant";
        CoutCapture cap;
        std::string s{pEx};
        print_ip(s);
        EXPECT_EQ(cap.str(), std::string(pEx) + '\n');
    }
    {
        const char* pEx = "4815162342->LOST...Dharma Initiative";
        CoutCapture cap;
        const std::string cs{pEx};
        print_ip(cs);
        EXPECT_EQ(cap.str(), std::string(pEx) + '\n');
    }
}

TEST(PrintIp, VectorAndListCases) 
{
    {
        CoutCapture cap;
        print_ip(std::vector<int>{100, 200, 300, 400});
        EXPECT_EQ(cap.str(), std::string("100.200.300.400\n"));
    }
    {
        CoutCapture cap;
        print_ip(std::list<short>{400, 300, 200, 100});
        EXPECT_EQ(cap.str(), std::string("400.300.200.100\n"));
    }
}

TEST(PrintIp, TupleUniformCase) 
{
    CoutCapture cap;
    print_ip(std::make_tuple(123, 456, 789, 0));
    EXPECT_EQ(cap.str(), std::string("123.456.789.0\n"));
}

// Compile-time checks for tuple heterogeneity (mirror enablement condition)
static_assert(uniform_tuple<std::tuple<int,int,int>>::value, "uniform tuple should be printable");
static_assert(!uniform_tuple<std::tuple<int, short>>::value, "heterogeneous tuple should NOT be printable");

