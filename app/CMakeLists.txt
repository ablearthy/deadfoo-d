add_executable(deadfoo-d-exe
        main.cc)

target_link_libraries(deadfoo-d-exe PRIVATE deadfoo-d-libs readline)