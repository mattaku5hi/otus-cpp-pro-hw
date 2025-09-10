#include <gtest/gtest.h>

#include "lib_version.h"


TEST(TestVersion, test_valid_version) {
    EXPECT_GT(version(), 0);
}


// Not used indeed as we have linked ::gtest_main in CMakeLists.txt
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}