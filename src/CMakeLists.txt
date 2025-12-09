set(PROJECT_NAME "mapreduce")
set(PROJECT_LIB "mapreduce_core")

# Настройка версии
configure_file(version.h.in version.h @ONLY)

# Основная библиотека
add_library(${PROJECT_LIB} lib_version.cpp csv_utils.cpp)
add_library(${PROJECT_NAME}::lib ALIAS ${PROJECT_LIB})
set_target_properties(${PROJECT_LIB} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
target_include_directories(${PROJECT_LIB}
    PRIVATE "${CMAKE_BINARY_DIR}"
    PRIVATE "${CMAKE_BINARY_DIR}/src"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Исполняемый файл
add_executable(${PROJECT_NAME} main.cpp)

# Установка стандартов
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

add_custom_target(${PROJECT_NAME}_app DEPENDS ${PROJECT_NAME})

# Линковка
target_link_libraries(${PROJECT_NAME} PRIVATE mapreduce::lib)

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


add_executable(mapper_mean mapper_mean.cpp)
add_executable(reducer_mean reducer_mean.cpp)
add_executable(mapper_var mapper_var.cpp)
add_executable(reducer_var reducer_var.cpp)

# Link tools with core
foreach(t IN ITEMS mapper_mean reducer_mean mapper_var reducer_var)
  target_link_libraries(${t} PRIVATE mapreduce_core)
  set_target_properties(${t} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)
endforeach()

# Install only the four main utilities into the package
install(TARGETS mapper_mean reducer_mean mapper_var reducer_var
    RUNTIME DESTINATION bin)
