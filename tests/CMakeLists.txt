add_subdirectory(${CMAKE_SOURCE_DIR}/deps/doctest doctest)

add_executable(tests
    main.cpp

    math/matrix.cpp
    math/vector.cpp
    math/point.cpp

    render/scheduler.cpp
    render/tile.cpp
)

target_link_libraries(tests
    PRIVATE
        rays
        doctest::doctest
)

add_test(NAME tests COMMAND tests)
