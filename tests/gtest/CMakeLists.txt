set(PROJECT_NAME "gtest_tests")

# Try the package first (system-installed or prebuilt)
find_package(GTest QUIET)

if(NOT TARGET GTest::gtest OR NOT TARGET GTest::gtest_main)
    # Fallback to submodule. Since the source dir is outside the current dir,
    # we must provide an explicit binary dir.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    add_subdirectory(
        ${CMAKE_SOURCE_DIR}/third_party/googletest
        ${CMAKE_BINARY_DIR}/third_party/googletest
    )

    # Provide namespaced aliases so existing code using GTest::gtest[_main] still works
    if(TARGET gtest AND NOT TARGET GTest::gtest)
        add_library(GTest::gtest ALIAS gtest)
    endif()
    if(TARGET gtest_main AND NOT TARGET GTest::gtest_main)
        add_library(GTest::gtest_main ALIAS gtest_main)
    endif()
    if(TARGET gmock AND NOT TARGET GTest::gmock)
        add_library(GTest::gmock ALIAS gmock)
    endif()
    if(TARGET gmock_main AND NOT TARGET GTest::gmock_main)
        add_library(GTest::gmock_main ALIAS gmock_main)
    endif()
endif()

# Simple Map tests removed

# --- Arena Allocator tests ---
add_executable(gtest_arena_allocator
    test_arena_allocator.cpp
)
target_include_directories(gtest_arena_allocator PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(gtest_arena_allocator PRIVATE 
    GTest::gtest 
    GTest::gtest_main 
    simple_allocator::lib
)

set_target_properties(gtest_arena_allocator PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(gtest_arena_allocator PRIVATE /W4)
else()
    target_compile_options(gtest_arena_allocator PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# --- Version tests (separate executable) ---
add_executable(gtest_version
    test_version.cpp
)
target_include_directories(gtest_version PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(gtest_version PRIVATE 
    GTest::gtest 
    GTest::gtest_main 
    simple_allocator::lib
)
set_target_properties(gtest_version PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
if(MSVC)
    target_compile_options(gtest_version PRIVATE /W4)
else()
    target_compile_options(gtest_version PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# Do not install test executables

enable_testing()
add_test(NAME gtest_arena_allocator COMMAND gtest_arena_allocator)
add_test(NAME gtest_version COMMAND gtest_version)

# --- Simple Vector tests ---
add_executable(gtest_simple_vector
    test_simple_vector.cpp
)
target_include_directories(gtest_simple_vector PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(gtest_simple_vector PRIVATE 
    GTest::gtest 
    GTest::gtest_main 
    simple_allocator::lib
)
set_target_properties(gtest_simple_vector PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
if(MSVC)
    target_compile_options(gtest_simple_vector PRIVATE /W4)
else()
    target_compile_options(gtest_simple_vector PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_test(NAME gtest_simple_vector COMMAND gtest_simple_vector)
