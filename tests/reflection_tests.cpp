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

refl::type<Foo> refl_foo{refl_field(i), refl_field(d)};
refl::type<Bar> refl_bar{refl_field(foo), refl_field(str)};

TEST(test_refl, should_successfully_store_field_info) {
    ASSERT_EQ(refl_foo.fields.size(), 2);
    ASSERT_EQ(refl_foo.fields[0]->get_name(), "i");
    ASSERT_EQ(refl_foo.fields[1]->get_name(), "d");

    ASSERT_EQ(refl_bar.fields.size(), 2);
    ASSERT_EQ(refl_bar.fields[0]->get_name(), "foo");
    ASSERT_EQ(refl_bar.fields[1]->get_name(), "str");

    // should successfully set foo and bar instance value
    Foo f{};
    refl_foo.set_field_value(f, "i", 10);
    refl_foo.set_field_value(f, "d", 3.14);

    Bar b{};
    refl_bar.set_field_value(b, "foo", Foo{10, 3.14});
    refl_bar.set_field_value(b, "str", std::string{"xxx"});

    ASSERT_EQ(f.i, 10);
    ASSERT_EQ(f.d, 3.14);
    ASSERT_EQ(b.foo.i, 10);
    ASSERT_EQ(b.foo.d, 3.14);
    ASSERT_EQ(b.str, "xxx");
}