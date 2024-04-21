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