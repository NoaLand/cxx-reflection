add_library(
        cxx_reflection_lib
        cxx_reflection.cpp
)
target_include_directories(cxx_reflection_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
