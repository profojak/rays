set(SDL_DIR ${CMAKE_SOURCE_DIR}/deps/SDL)
set(SDL_TEST OFF)
set(SDL_INSTALL_TESTS OFF)

add_subdirectory(${SDL_DIR} ${CMAKE_BINARY_DIR}/deps/SDL EXCLUDE_FROM_ALL)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/deps/ImGUI)

file(GLOB IMGUI_CORE_SOURCES ${IMGUI_DIR}/*.cpp)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.h
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.h
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.h
)

add_executable(gui
    main.cpp
    ${IMGUI_CORE_SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(gui
    PRIVATE
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(gui
    PRIVATE
        rays
        SDL3::SDL3
)
