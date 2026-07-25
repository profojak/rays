add_library(rays STATIC)

target_sources(rays
    PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        FILES
            rays.cppm
)

target_sources(rays
    PRIVATE
        ../core/api.cpp
)

target_include_directories(rays
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
