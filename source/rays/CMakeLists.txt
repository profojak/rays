add_library(rays STATIC)

target_sources(rays
    PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        FILES
            rays.cppm

            camera/camera.cppm

            image/film.cppm
            image/image.cppm
            image/pixel.cppm

            integrator/integrator.cppm
            integrator/montecarlo.cppm

            math/bounds.cppm
            math/point.cppm
            math/vector.cppm

            render/scheduler.cppm
            render/tile.cppm

            state/state.cppm
            state/thread.cppm

            utility/type.cppm

        FILE_SET headers
        TYPE HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            rays.hpp
            math/math.hpp
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
