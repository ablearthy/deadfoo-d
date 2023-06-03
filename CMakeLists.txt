cmake_minimum_required(VERSION 3.23)

project(deadfoo-d
        VERSION 0.0.1
        LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else ()
    # additional warnings
    add_compile_options(
            -Wall -Wextra -Wshadow -Wpedantic
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wconversion
            -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wuseless-cast
    )
endif ()

add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(tests)