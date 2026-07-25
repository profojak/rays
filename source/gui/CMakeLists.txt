add_executable(gui
    main.cpp
)

target_link_libraries(gui
    PRIVATE
        rays
)
