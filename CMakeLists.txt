cmake_minimum_required(VERSION 3.20)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(ip_filter VERSION ${PROJECT_VERSION} LANGUAGES CXX)

option(ENABLE_BOOST_TEST "Whether to build Boost test" OFF)
option(ENABLE_GOOGLE_TEST "Whether to build Google test" ON)

# Отключаем установку в системные директории
if(${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT})
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install path prefix" FORCE)
endif()

# Добавление поддиректорий
add_subdirectory(src)

if(ENABLE_BOOST_TEST OR ENABLE_GOOGLE_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()

# Настройка CPack
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT mattakushi@gmail.com)
include(CPack)
