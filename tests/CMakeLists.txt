include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.12.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
        cpp_project_template_tests
        test_main.cpp
        test_template.cpp
)

target_link_libraries(
        cpp_project_template_tests
        cpp_project_template_lib
        gtest_main
        gmock_main
)
