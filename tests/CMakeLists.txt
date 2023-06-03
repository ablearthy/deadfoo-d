include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(deadfoo-d-libs-test main.cc)

target_link_libraries(deadfoo-d-libs-test
        GTest::gtest_main
        deadfoo-d-libs)


include(GoogleTest)
gtest_discover_tests(deadfoo-d-libs-test)
