#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE TEST_VERSION
#endif

#include <boost/test/included/unit_test.hpp>
#include "lib_version.h"


BOOST_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(test_valid_version) {
	BOOST_CHECK(version() > 0);
}

BOOST_AUTO_TEST_SUITE_END()
