
#include <boost/test/unit_test.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "ip_tools.h"


using ip_filter::IPv4;


BOOST_AUTO_TEST_SUITE(IpTools)


BOOST_AUTO_TEST_CASE(Split_BasicDotSeparated)
{
    const std::string s = "11.22.33.44";
    const auto parts = ip_filter::split(s, '.');
    BOOST_REQUIRE_EQUAL(parts.size(), 4u);
    BOOST_CHECK(parts[0] == "11");
    BOOST_CHECK(parts[1] == "22");
    BOOST_CHECK(parts[2] == "33");
    BOOST_CHECK(parts[3] == "44");
}

BOOST_AUTO_TEST_CASE(ParseIPv4_Valid)
{
    const auto ip = ip_filter::ParseIpv4("1.2.3.4");
    BOOST_CHECK(ip == (IPv4{1,2,3,4}));
}

BOOST_AUTO_TEST_CASE(ParseIPv4_InvalidWrongParts)
{
    BOOST_CHECK_THROW(ip_filter::ParseIpv4("1.2.3"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(ParseIPv4_InvalidEmptyOctet)
{
    BOOST_CHECK_THROW(ip_filter::ParseIpv4("1..3.4"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(ParseIPv4_InvalidNonInteger)
{
    BOOST_CHECK_THROW(ip_filter::ParseIpv4("a.2.3.4"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(ParseIPv4_InvalidOutOfRange)
{
    BOOST_CHECK_THROW(ip_filter::ParseIpv4("256.2.3.4"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(ReadInputIPs_ReadsFirstColumnAndSkipsEmpty)
{
    std::istringstream in(
        "1.2.3.4\tcomment1\n"
        "\n"
        "2.3.4.5\tcomment2\n"
    );
    const auto ips = ip_filter::ReadInputIps(in);
    BOOST_REQUIRE_EQUAL(ips.size(), 2u);
    BOOST_CHECK(ips[0] == (IPv4{1,2,3,4}));
    BOOST_CHECK(ips[1] == (IPv4{2,3,4,5}));
}

BOOST_AUTO_TEST_CASE(SortIPsDesc_SortsLexicographicallyDescending)
{
    std::vector<IPv4> ips{
        IPv4{1,2,3,4},
        IPv4{1,1,1,1},
        IPv4{2,0,0,0},
        IPv4{1,2,4,0}
    };
    ip_filter::SortIpsDesc(ips);
    BOOST_REQUIRE_EQUAL(ips.size(), 4u);
    BOOST_CHECK(ips[0] == (IPv4{2,0,0,0}));
    BOOST_CHECK(ips[1] == (IPv4{1,2,4,0}));
    BOOST_CHECK(ips[2] == (IPv4{1,2,3,4}));
    BOOST_CHECK(ips[3] == (IPv4{1,1,1,1}));
}

BOOST_AUTO_TEST_CASE(PrintIp_PrintsSingleIp)
{
    std::ostringstream out;
    ip_filter::PrintIp(IPv4{10,20,30,40}, out);
    BOOST_CHECK(out.str() == std::string("10.20.30.40\n"));
}

BOOST_AUTO_TEST_CASE(PrintIp_PrintsContainerOfIps)
{
    std::ostringstream out;
    std::vector<IPv4> ips{ IPv4{1,2,3,4}, IPv4{5,6,7,8} };
    ip_filter::PrintIp(ips, out);
    BOOST_CHECK(out.str() == std::string("1.2.3.4\n5.6.7.8\n"));
}

BOOST_AUTO_TEST_CASE(Filters_SecondThirdFourth)
{
    const std::vector<IPv4> ips{
        IPv4{1,2,3,4},
        IPv4{46,70,1,2},
        IPv4{46,1,70,2},
        IPv4{7,46,8,9},
        IPv4{1,1,1,46}
    };

    const auto f1 = ip_filter::FilterSecondStep(ips, 1);
    BOOST_REQUIRE_EQUAL(f1.size(), 2u);
    BOOST_CHECK(f1[0] == (IPv4{1,2,3,4}));
    BOOST_CHECK(f1[1] == (IPv4{1,1,1,46}));

    const auto f46_70 = ip_filter::FilterThirdStep(ips, 46, 70);
    BOOST_REQUIRE_EQUAL(f46_70.size(), 1u);
    BOOST_CHECK(f46_70[0] == (IPv4{46,70,1,2}));

    const auto any46 = ip_filter::FilterFourthStep(ips, 46);
    BOOST_REQUIRE_EQUAL(any46.size(), 4u);
    BOOST_CHECK(any46[0] == (IPv4{46,70,1,2}));
    BOOST_CHECK(any46[1] == (IPv4{46,1,70,2}));
    BOOST_CHECK(any46[2] == (IPv4{7,46,8,9}));
    BOOST_CHECK(any46[3] == (IPv4{1,1,1,46}));
}

BOOST_AUTO_TEST_SUITE_END()
