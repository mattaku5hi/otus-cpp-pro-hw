
add_executable(fashion_mnist
    fashion_mnist.cpp
    csv_parser.cpp
)

set_target_properties(fashion_mnist PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(fashion_mnist PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

if (MSVC)
    target_compile_options(fashion_mnist PRIVATE /W4)
else()
    target_compile_options(fashion_mnist PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS fashion_mnist
    RUNTIME DESTINATION bin
)

option(ENABLE_FASHION_MNIST_TF "Build fashion_mnist_tf (TensorFlow SavedModel inference)" ON)
set(TF_CAPI_INC_DIR "" CACHE PATH "Optional override: TensorFlow C API include dir (contains tensorflow/c/c_api.h)")
set(TF_CAPI_LIB_DIR "" CACHE PATH "Optional override: TensorFlow C API lib dir (contains libtensorflow.so)")

if (ENABLE_FASHION_MNIST_TF)
    find_path(TENSORFLOW_CAPI_INCLUDE_DIR
        NAMES tensorflow/c/c_api.h
        HINTS ${TF_CAPI_INC_DIR}
    )
    find_library(TENSORFLOW_CAPI_LIB
        NAMES tensorflow libtensorflow
        HINTS ${TF_CAPI_LIB_DIR}
    )

    if (TENSORFLOW_CAPI_INCLUDE_DIR AND TENSORFLOW_CAPI_LIB)
        add_executable(fashion_mnist_tf 
            fashion_mnist_tf.cpp
            csv_parser.cpp
        )
        target_include_directories(fashion_mnist_tf PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${TENSORFLOW_CAPI_INCLUDE_DIR}")
        target_link_libraries(fashion_mnist_tf PRIVATE ${TENSORFLOW_CAPI_LIB})
        set_target_properties(fashion_mnist_tf PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
        )
        if (TF_CAPI_LIB_DIR)
            set_target_properties(fashion_mnist_tf PROPERTIES
                BUILD_RPATH "${TF_CAPI_LIB_DIR}"
                INSTALL_RPATH "${TF_CAPI_LIB_DIR}"
            )
        endif()
        if (MSVC)
            target_compile_options(fashion_mnist_tf PRIVATE /W4)
        else()
            target_compile_options(fashion_mnist_tf PRIVATE -Wall -Wextra -pedantic -Werror)
        endif()
        install(TARGETS fashion_mnist_tf
            RUNTIME DESTINATION bin
        )
    else()
        message(WARNING "ENABLE_FASHION_MNIST_TF is ON but TensorFlow C API was not found. Install libtensorflow (C API) or set TF_CAPI_INC_DIR/TF_CAPI_LIB_DIR.")
    endif()
endif()
