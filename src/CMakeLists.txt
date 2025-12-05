set(PROJECT_NAME "join_server")
set(PROJECT_LIB "join_server_core")

# Настройка версии
configure_file(version/version.h.in version/version.h @ONLY)

# Основная библиотека
add_library(${PROJECT_LIB} version/lib_version.cpp)
add_library(join_server::lib ALIAS ${PROJECT_LIB})
set_target_properties(${PROJECT_LIB} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_LIB}
    PRIVATE "${CMAKE_BINARY_DIR}"
    PRIVATE "${CMAKE_BINARY_DIR}/src"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Исполняемый файл
add_executable(${PROJECT_NAME} join_server.cpp)

# Установка стандартов
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

add_custom_target(${PROJECT_NAME}_app DEPENDS ${PROJECT_NAME})

# Линковка
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

target_link_libraries(${PROJECT_NAME} PRIVATE join_server::lib Threads::Threads Boost::system)

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
