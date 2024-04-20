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

refl::type<Foo, decltype(Foo::i), decltype(Foo::d)> refl_foo{refl_field(i), refl_field(d)};
refl::type<Bar, decltype(Bar::foo), decltype(Bar::str)> refl_bar{refl_field(foo), refl_field(str)};

TEST(refl_type_system_test, should_succesfully_create_variant_types_for_class_after_using_refl_type) {
    ASSERT_TRUE((std::is_same_v<std::variant<refl::type_identity<int>, refl::type_identity<double>>, typename decltype(refl_foo)::field_types_variant>));
    ASSERT_TRUE((std::is_same_v<std::variant<refl::type_identity<Foo>, refl::type_identity<std::string>>, typename decltype(refl_bar)::field_types_variant>));
}

TEST(refl_type_system_test, should_successfully_store_field_info_after_using_refl_type) {
    ASSERT_EQ(refl_foo.fields.size(), 2);
    ASSERT_EQ(refl_foo.fields[0]->get_name(), "i");
    ASSERT_EQ(refl_foo.fields[1]->get_name(), "d");

    ASSERT_EQ(refl_bar.fields.size(), 2);
    ASSERT_EQ(refl_bar.fields[0]->get_name(), "foo");
    ASSERT_EQ(refl_bar.fields[1]->get_name(), "str");
}

TEST(refl_type_system_test, should_successfully_set_field_value_when_passing_instance_and_field_name_and_value) {
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