
set(PROJECT_NAME "editor")
set(PROJECT_LIB "editor_core")

# Настройка версии
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version/version.h.in" 
    "${CMAKE_CURRENT_SOURCE_DIR}/version/version.h" 
    @ONLY
)

# Основная библиотека
add_library(${PROJECT_LIB}
    "${CMAKE_CURRENT_SOURCE_DIR}/commands/commands.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/id_generator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ctrl/editor_controller.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/model/document.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/model/serializer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/model/shapes/line.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/model/shapes/rectangle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/model/shapes/shape_factory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/version/lib_version.cpp"
)

add_library(editor::lib ALIAS ${PROJECT_LIB})
set_target_properties(${PROJECT_LIB} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
target_include_directories(${PROJECT_LIB}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/commands"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/common"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ctrl"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/model"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/model/shapes"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/version"
)

# Исполняемый файл
add_executable(${PROJECT_NAME} 
    "${CMAKE_CURRENT_SOURCE_DIR}/editor.cpp"
)

# Установка стандартов
set_target_properties(${PROJECT_NAME} ${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

add_custom_target(${PROJECT_NAME}_app DEPENDS ${PROJECT_NAME})

# Линковка
target_link_libraries(${PROJECT_NAME} PRIVATE editor::lib)

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