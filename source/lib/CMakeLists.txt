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
)

target_sources(rays
    PRIVATE
        ../core/api.cpp
        ../core/linalg.cpp
        ../core/camera/camera.cpp
        ../core/camera/film.cpp
)

target_include_directories(rays
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(rays
    PRIVATE
        ${CMAKE_BINARY_DIR}/deps/stdBLAS/include/experimental
)

target_link_libraries(rays
    PRIVATE
        std::linalg
)
