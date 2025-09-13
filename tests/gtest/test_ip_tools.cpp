#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

#include "ip_tools.h"

using ip_filter::IPv4;


TEST(IpTools_Split, BasicDotSeparated)
{
    const std::string s = "11.22.33.44";
    const auto parts = ip_filter::split(s, '.');
    ASSERT_EQ(parts.size(), 4u);
    EXPECT_EQ(parts[0], "11");
    EXPECT_EQ(parts[1], "22");
    EXPECT_EQ(parts[2], "33");
    EXPECT_EQ(parts[3], "44");
}

TEST(IpTools_ParseIPv4, Valid)
{
    const auto ip = ip_filter::ParseIpv4("1.2.3.4");
    EXPECT_EQ(ip[0], 1);
    EXPECT_EQ(ip[1], 2);
    EXPECT_EQ(ip[2], 3);
    EXPECT_EQ(ip[3], 4);
}

TEST(IpTools_ParseIPv4, InvalidWrongParts)
{
    EXPECT_THROW(ip_filter::ParseIpv4("1.2.3"), std::runtime_error);
}

TEST(IpTools_ParseIPv4, InvalidEmptyOctet)
{
    EXPECT_THROW(ip_filter::ParseIpv4("1..3.4"), std::runtime_error);
}

TEST(IpTools_ParseIPv4, InvalidNonInteger)
{
    EXPECT_THROW(ip_filter::ParseIpv4("a.2.3.4"), std::runtime_error);
}

TEST(IpTools_ParseIPv4, InvalidOutOfRange)
{
    EXPECT_THROW(ip_filter::ParseIpv4("256.2.3.4"), std::runtime_error);
}

TEST(IpTools_ReadInputIPs, ReadsFirstColumnAndSkipsEmpty)
{
    std::istringstream in(
        "1.2.3.4\tcomment1\n"
        "\n"
        "2.3.4.5\tcomment2\n"
    );
    const auto ips = ip_filter::ReadInputIps(in);
    ASSERT_EQ(ips.size(), 2u);
    EXPECT_EQ(ips[0], (IPv4{1,2,3,4}));
    EXPECT_EQ(ips[1], (IPv4{2,3,4,5}));
}

TEST(IpTools_SortIPsDesc, SortsLexicographicallyDescending)
{
    std::vector<IPv4> ips{
        IPv4{1,2,3,4},
        IPv4{1,1,1,1},
        IPv4{2,0,0,0},
        IPv4{1,2,4,0}
    };
    ip_filter::SortIpsDesc(ips);
    // Expected order: 2.0.0.0, 1.2.4.0, 1.2.3.4, 1.1.1.1
    ASSERT_EQ(ips.size(), 4u);
    EXPECT_EQ(ips[0], (IPv4{2,0,0,0}));
    EXPECT_EQ(ips[1], (IPv4{1,2,4,0}));
    EXPECT_EQ(ips[2], (IPv4{1,2,3,4}));
    EXPECT_EQ(ips[3], (IPv4{1,1,1,1}));
}

TEST(IpTools_PrintIp, PrintsSingleIp)
{
    std::ostringstream out;
    ip_filter::PrintIp(IPv4{10,20,30,40}, out);
    EXPECT_EQ(out.str(), std::string("10.20.30.40\n"));
}

TEST(IpTools_PrintIp, PrintsContainerOfIps)
{
    std::ostringstream out;
    std::vector<IPv4> ips{ IPv4{1,2,3,4}, IPv4{5,6,7,8} };
    ip_filter::PrintIp(ips, out);
    EXPECT_EQ(out.str(), std::string("1.2.3.4\n5.6.7.8\n"));
}

TEST(IpTools_Filters, SecondThirdFourth)
{
    const std::vector<IPv4> ips{
        IPv4{1,2,3,4},
        IPv4{46,70,1,2},
        IPv4{46,1,70,2},
        IPv4{7,46,8,9},
        IPv4{1,1,1,46}
    };

    const auto f1 = ip_filter::FilterSecondStep(ips, 1);
    ASSERT_EQ(f1.size(), 2u);
    EXPECT_EQ(f1[0], (IPv4{1,2,3,4}));
    EXPECT_EQ(f1[1], (IPv4{1,1,1,46}));

    const auto f46_70 = ip_filter::FilterThirdStep(ips, 46, 70);
    ASSERT_EQ(f46_70.size(), 1u);
    EXPECT_EQ(f46_70[0], (IPv4{46,70,1,2}));

    const auto any46 = ip_filter::FilterFourthStep(ips, 46);
    ASSERT_EQ(any46.size(), 4u);
    EXPECT_EQ(any46[0], (IPv4{46,70,1,2}));
    EXPECT_EQ(any46[1], (IPv4{46,1,70,2}));
    EXPECT_EQ(any46[2], (IPv4{7,46,8,9}));
    EXPECT_EQ(any46[3], (IPv4{1,1,1,46}));
}
