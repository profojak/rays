add_executable(gui
    main.cpp
)

set(SDL_DIR ${CMAKE_SOURCE_DIR}/deps/SDL)
set(SDL_TEST OFF)
set(SDL_INSTALL_TESTS OFF)

add_subdirectory(${SDL_DIR} ${CMAKE_BINARY_DIR}/deps/SDL EXCLUDE_FROM_ALL)

target_link_libraries(gui
    PRIVATE
        rays
        SDL3::SDL3
)
