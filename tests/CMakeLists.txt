include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.12.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
        cxx_reflection_tests
        test_main.cpp
        reflection_tests.cpp
        noaland_lib_tests.cpp
)

target_link_libraries(
        cxx_reflection_tests
        cxx_reflection_lib
        gtest_main
        gmock_main
)
