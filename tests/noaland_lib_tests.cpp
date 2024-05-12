#include "gtest/gtest.h"

#include "noaland_lib.h"

#include <vector>

struct Foo {
    int i;
    double d;
    float f;
};

TEST(is_fuzzy_type_matched_test, should_successfully_detect_two_same_types_std) {
    static_assert(noaland::is_fuzzy_type_matched_v<int, int>);

    static_assert(noaland::is_fuzzy_type_matched_v<Foo, Foo>);

    static_assert(noaland::is_fuzzy_type_matched_v<std::vector<int>, std::vector<int>>);
}

TEST(is_fuzzy_type_matched_test, should_successfully_detect_one_type_with_i_dont_care) {
    static_assert(noaland::is_fuzzy_type_matched_v<int, noaland::i_dont_care>);
    static_assert(noaland::is_fuzzy_type_matched_v<noaland::i_dont_care, int>);

    static_assert(noaland::is_fuzzy_type_matched_v<Foo, noaland::i_dont_care>);
    static_assert(noaland::is_fuzzy_type_matched_v<noaland::i_dont_care, Foo>);
}

TEST(is_fuzzy_type_matched_test, should_successfully_detect_vector_with_i_dont_care_is_a_vector) {
    static_assert(noaland::is_fuzzy_type_matched_v<std::vector<int>, std::vector<noaland::i_dont_care>>);
    static_assert(noaland::is_fuzzy_type_matched_v<std::vector<Foo>, std::vector<noaland::i_dont_care>>);
    static_assert(noaland::is_fuzzy_type_matched_v<std::vector<Foo*>, std::vector<noaland::i_dont_care>>);
}

template<typename T>
struct some_template {};

TEST(is_fuzzy_type_matched_test, should_successfully_detect_template_of_something_with_i_dont_care) {
    static_assert(noaland::is_fuzzy_type_matched_v<some_template<some_template<int>>, some_template<noaland::i_dont_care>>);
    static_assert(noaland::is_fuzzy_type_matched_v<some_template<some_template<int>>, some_template<some_template<noaland::i_dont_care>>>);
}

template<typename... T>
struct some_variadic_template {};

TEST(is_fuzzy_type_matched_test, should_successfully_detect_template_of_variadic_param_with_i_dont_care) {
    static_assert(noaland::is_fuzzy_type_matched_v<some_variadic_template<int, float, some_variadic_template<double>>, some_variadic_template<int, float, noaland::i_dont_care>>);
    static_assert(noaland::is_fuzzy_type_matched_v<some_variadic_template<int, float, some_variadic_template<double>>, some_variadic_template<noaland::i_dont_care, float, some_variadic_template<double>>>);
}

TEST(is_fuzzy_type_matched_test, should_get_false_when_two_types_are_different) {
    static_assert(!noaland::is_fuzzy_type_matched_v<some_variadic_template<int, float, some_variadic_template<double>>, some_variadic_template<noaland::i_dont_care, double, some_variadic_template<double>>>);
    static_assert(noaland::is_fuzzy_type_matched_v<std::vector<std::vector<std::vector<noaland::i_dont_care>>>, std::vector<noaland::i_dont_care>>);
}