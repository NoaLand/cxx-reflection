#include "gtest/gtest.h"

#include <string>

#include "cxx_reflection.h"

class Foo {
public:
    int i;
    double d;
};

class Bar {
public:
    Foo foo;
    std::string str;
};

refl::type<Foo> foo{refl_field(i), refl_field(d)};
refl::type<Bar> bar{refl_field(foo), refl_field(str)};


TEST(test_template, test_should_pass_when_1_is_equal_to_1) {
    ASSERT_EQ(foo.fields.size(), 2);
    ASSERT_EQ(foo.fields[0]->get_name(), "i");
    ASSERT_EQ(foo.fields[1]->get_name(), "d");

    ASSERT_EQ(bar.fields.size(), 2);
    ASSERT_EQ(bar.fields[0]->get_name(), "foo");
    ASSERT_EQ(bar.fields[1]->get_name(), "str");
}