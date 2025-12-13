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
find_package(SQLite3 REQUIRED)

if(TARGET SQLite3::SQLite3)
    target_link_libraries(${PROJECT_NAME} PRIVATE join_server::lib Threads::Threads Boost::system SQLite3::SQLite3)
elseif(TARGET SQLite::SQLite3)
    target_link_libraries(${PROJECT_NAME} PRIVATE join_server::lib Threads::Threads Boost::system SQLite::SQLite3)
else()
    # Fallback for older FindSQLite3.cmake implementations that don't define an imported target.
    if(SQLite3_INCLUDE_DIR)
        target_include_directories(${PROJECT_NAME} PRIVATE ${SQLite3_INCLUDE_DIR})
    elseif(SQLite3_INCLUDE_DIRS)
        target_include_directories(${PROJECT_NAME} PRIVATE ${SQLite3_INCLUDE_DIRS})
    endif()

    if(SQLite3_LIBRARY)
        target_link_libraries(${PROJECT_NAME} PRIVATE join_server::lib Threads::Threads Boost::system ${SQLite3_LIBRARY})
    elseif(SQLite3_LIBRARIES)
        target_link_libraries(${PROJECT_NAME} PRIVATE join_server::lib Threads::Threads Boost::system ${SQLite3_LIBRARIES})
    else()
        message(FATAL_ERROR "SQLite3 was found but no library target/variable is available to link")
    endif()
endif()

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
