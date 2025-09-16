cmake_minimum_required(VERSION 3.20)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(simple_allocator VERSION ${PROJECT_VERSION} LANGUAGES CXX)

option(ENABLE_BOOST_TEST "Whether to build Boost test" OFF)
option(ENABLE_GOOGLE_TEST "Whether to build Google test" ON)

# Always enable CTest at top-level so `ctest --test-dir <build>` works
include(CTest)
enable_testing()

# Отключаем установку в системные директории
if(${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT})
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install path prefix" FORCE)
endif()

# Добавление поддиректорий
add_subdirectory(src)

# Add test subdirectories only if requested, but keep top-level CTest enabled
if(ENABLE_BOOST_TEST OR ENABLE_GOOGLE_TEST)
    add_subdirectory(tests)
endif()

# Настройка CPack
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT mattakushi@gmail.com)
include(CPack)
