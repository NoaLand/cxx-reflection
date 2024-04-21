#include "gtest/gtest.h"

#include "noaland_lib.h"

#include <vector>

struct Foo {
    int i;
    double d;
    float f;
};

TEST(is_a_test, should_successfully_detect_two_same_types_std) {
    ASSERT_TRUE((noaland::is_a_v<int, int>));

    ASSERT_TRUE((noaland::is_a_v<Foo, Foo>));

    ASSERT_TRUE((noaland::is_a_v<std::vector<int>, std::vector<int>>));
}

TEST(is_a_test, should_successfully_detect_one_type_with_i_dont_care) {
    ASSERT_TRUE((noaland::is_a_v<int, noaland::i_dont_care>));
    ASSERT_TRUE((noaland::is_a_v<noaland::i_dont_care, int>));

    ASSERT_TRUE((noaland::is_a_v<Foo, noaland::i_dont_care>));
    ASSERT_TRUE((noaland::is_a_v<noaland::i_dont_care, Foo>));
}

TEST(is_a_test, should_successfully_detect_vector_with_i_dont_care_is_a_vector) {
    ASSERT_TRUE((noaland::is_a_v<std::vector<int>, std::vector<noaland::i_dont_care>>));
    ASSERT_TRUE((noaland::is_a_v<std::vector<Foo>, std::vector<noaland::i_dont_care>>));
    ASSERT_TRUE((noaland::is_a_v<std::vector<Foo*>, std::vector<noaland::i_dont_care>>));
}

template<typename T>
struct some_template {};

TEST(is_a_test, should_successfully_detect_template_of_something_with_i_dont_care) {
    ASSERT_TRUE((noaland::is_a_v<some_template<some_template<int>>, some_template<noaland::i_dont_care>>));
    ASSERT_TRUE((noaland::is_a_v<some_template<some_template<int>>, some_template<some_template<noaland::i_dont_care>>>));
}

template<typename... T>
struct some_variadic_template {};

TEST(is_a_test, should_successfully_detect_template_of_variadic_param_with_i_dont_care) {
    ASSERT_TRUE((noaland::is_a_v<some_variadic_template<int, float, some_variadic_template<double>>, some_variadic_template<int, float, noaland::i_dont_care>>));
    ASSERT_TRUE((noaland::is_a_v<some_variadic_template<int, float, some_variadic_template<double>>, some_variadic_template<noaland::i_dont_care, float, some_variadic_template<double>>>));
}

TEST(is_a_test, should_get_false_when_two_types_are_different) {
    ASSERT_FALSE((noaland::is_a_v<some_variadic_template<int, float, some_variadic_template<double>>, some_variadic_template<noaland::i_dont_care, double, some_variadic_template<double>>>));
}