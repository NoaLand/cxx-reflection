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

noaland::expected<int, std::string> get_value(int x) {
    if (x > 0) {
        return x;
    } else {
        return noaland::unexpected(std::string("x is less than 0"));
    }
}

TEST(expected_test, should_successfully_get_value_when_using_expected) {
    auto v = get_value(10)
        .and_should([](int val){ return val % 2 == 0; }, std::string("validation failed!"))
        .and_then([](int val){ return val + 1; })
        .or_else([](auto ex){ std::cout << ex.what() << '\n'; })
        .value_or(-1);

    ASSERT_EQ(v, 11);
}

TEST(expected_test, should_get_default_value_when_using_expected) {
    auto v = get_value(10)
        .and_should([](int val){ return val % 2 == 1; }, std::string("validation failed!"))
        .and_then([](int val){ return val + 1; })
        .or_else([](auto ex){ std::cout << "Exception: " << ex.what() << '\n'; })
        .value_or(-1);

    ASSERT_EQ(v, -1);
}