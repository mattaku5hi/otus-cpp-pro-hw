

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


add_executable(${PROJECT_NAME}
    test_version.cpp
    test_editor.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/ctrl"
    "${CMAKE_SOURCE_DIR}/src/model"
    "${CMAKE_SOURCE_DIR}/src/model/shapes"
    "${CMAKE_SOURCE_DIR}/src/version"
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
	GTest::gtest 
	GTest::gtest_main 
	editor::lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# Do not install test executables

enable_testing()
add_test(NAME gtest COMMAND ${PROJECT_NAME})
