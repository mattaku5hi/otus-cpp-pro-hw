
set(PROJECT_LIB "simple_allocator_core")
set(PROJECT_NAME "simple_allocator")

# Настройка версии
configure_file(version.h.in version.h @ONLY)

set(SOURCES_LIB
    arena_allocator.h
    lib_version.cpp
    simple_vector.h
)

set(SOURCES_EXE
    simple_allocator.cpp
)

# Основная библиотека
add_library(${PROJECT_LIB} ${SOURCES_LIB})
# Provide a clean alias for consumers without colliding with the executable target name
add_library(simple_allocator::lib ALIAS ${PROJECT_LIB})
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
add_custom_target(simple_allocator_app DEPENDS ${PROJECT_NAME})

# Установка стандартов
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Линковка
target_link_libraries(${PROJECT_NAME} PRIVATE simple_allocator::lib)

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