add_library(rays STATIC)

target_sources(rays
    PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        BASE_DIRS
            "${CMAKE_SOURCE_DIR}/source/lib"
            "${CMAKE_SOURCE_DIR}/source/core"
        FILES
            rays.cppm
            ../core/camera/camera.cppm
            ../core/camera/pixel.cpp
            ../core/camera/film.cpp
    PRIVATE
        ../core/api.cpp
        ../core/linalg.cpp
)

target_include_directories(rays
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_BINARY_DIR}/deps/stdBLAS/include/experimental
)

target_link_libraries(rays
    PRIVATE
        std::linalg
)
