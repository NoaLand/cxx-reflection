add_library(
        cxx_reflection_lib
        template.cpp
)
target_include_directories(cxx_reflection_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
