
set(PROJECT_NAME "print_ip")
set(PROJECT_LIB "print_ip_core")

# Настройка версии
configure_file(version.h.in version.h @ONLY)

# Основная библиотека
add_library(${PROJECT_LIB} lib_version.cpp print_ip.h)
add_library(print_ip::lib ALIAS ${PROJECT_LIB})
set_target_properties(${PROJECT_LIB} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
target_include_directories(${PROJECT_LIB}
    PRIVATE "${CMAKE_BINARY_DIR}"
    PRIVATE "${CMAKE_BINARY_DIR}/src"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Исполняемый файл
add_executable(${PROJECT_NAME} print_ip_app.cpp)

# Установка стандартов
set_target_properties(${PROJECT_NAME} ${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

add_custom_target(${PROJECT_NAME}_app DEPENDS ${PROJECT_NAME})

# Линковка
target_link_libraries(${PROJECT_NAME} PRIVATE print_ip::lib)

# Установка опций компилятора
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_options(${PROJECT_LIB} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(${PROJECT_LIB} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# Установка целей
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)