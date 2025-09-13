
set(PROJECT_LIB "ip_filter_core")
set(PROJECT_NAME "ip_filter")

# Настройка версии
configure_file(version.h.in version.h @ONLY)

set(SOURCES_LIB
    lib_version.cpp
)

set(SOURCES_EXE
    ip_tools.h
    ip_filter.cpp
)

# Основная библиотека
add_library(${PROJECT_LIB} ${SOURCES_LIB})
# Provide a clean alias for consumers without colliding with the executable target name
add_library(ipfilter::lib ALIAS ${PROJECT_LIB})
set_target_properties(${PROJECT_LIB} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
target_include_directories(${PROJECT_LIB}
    PRIVATE "${CMAKE_BINARY_DIR}"
    PRIVATE "${CMAKE_BINARY_DIR}/src"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCES_EXE})
target_include_directories(${PROJECT_NAME}
    PRIVATE "${CMAKE_BINARY_DIR}/src"
)

# Alias-like convenience target for the executable
# CMake does not support ALIAS for executables, so we create a phony target that depends on it
add_custom_target(ipfilter_app DEPENDS ${PROJECT_NAME})

# Установка стандартов
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Линковка
target_link_libraries(${PROJECT_NAME} PRIVATE ipfilter::lib)

# Установка опций компилятора
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# Установка целей
install(TARGETS ${PROJECT_LIB} ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
