
set(PROJECT_NAME "bulk")
set(PROJECT_LIB "bulk_core")

# Настройка версии
configure_file(version.h.in version.h @ONLY)

# Основная библиотека
add_library(${PROJECT_LIB}
    lib_version.cpp
    bulkapp/aggregator.cpp
    sinks/console_sink.cpp
    sinks/file_sink.cpp
)
add_library(bulk::lib ALIAS ${PROJECT_LIB})
set_target_properties(${PROJECT_LIB} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
set_target_properties(${PROJECT_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_LIB}
    # Generated headers like version.h
    PRIVATE "${CMAKE_BINARY_DIR}"
    PRIVATE "${CMAKE_BINARY_DIR}/src"
    # Project headers in source tree (use PUBLIC so consumers inherit)
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/bulkapp"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/model"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/sinks"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/async"
)

# Исполняемый файл
add_executable(${PROJECT_NAME}
    bulk.cpp
)

# Установка стандартов
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Включаем пути заголовков для исполняемой цели (минимально, остальное придёт из bulk::lib)
target_include_directories(${PROJECT_NAME}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE "${CMAKE_BINARY_DIR}"
    PRIVATE "${CMAKE_BINARY_DIR}/src"
)

add_custom_target(${PROJECT_NAME}_app DEPENDS ${PROJECT_NAME})

# Линковка
target_link_libraries(${PROJECT_NAME} PRIVATE async Threads::Threads)

# Установка опций компилятора
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(${PROJECT_LIB} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# Threads for async lib
find_package(Threads REQUIRED)

# Async shared library (libasync.so)
add_library(async SHARED
    async/async.cpp
    async/dispatcher.cpp
    async/dispatch_sink.cpp
)
set_target_properties(async PROPERTIES
    OUTPUT_NAME "async"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(async
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/async"
    PUBLIC "${CMAKE_BINARY_DIR}"
    PUBLIC "${CMAKE_BINARY_DIR}/src"
)
target_link_libraries(async
    PRIVATE bulk::lib
    PRIVATE Threads::Threads
)

# Install targets
install(TARGETS ${PROJECT_NAME} async
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

