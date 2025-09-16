#include <gtest/gtest.h>

#include "lib_version.h"


TEST(TestVersion, test_valid_version) {
    EXPECT_GT(version(), 0);
}

