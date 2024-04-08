# Open CXX Reflection

This is a C++ reflection library that is designed to be easy to use and is written in C++20.

And I'm trying to make it as simple as possible, so that user can use it without changing their implementation code.

BTW, in this library, I'll use template instead of macro.

## Syntax

```cpp
// for classes need to be reflected
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

// register them into the reflection system
refl::register<Foo> foo{refl::field<int>("i"), refl::field<double>("d")};
refl::register<Bar> bar{refl::field<Foo>("foo"), refl::field<std::string>("str")};

// use them in function
int main() {
    Bar bar{{1, 3.14}, "hello"};
    
    // should get field value by name
    auto str1 = refl::get_field<bar>("str");
    
    // should use type of field as type of variable,
    // create a new variable named "str2" with type std::string
    refl::type_of_field<bar>("str") str2{"world"};
    
    // create a new variable named "foo" with type Foo
    refl::type_of_field<bar>("foo") foo{2, 7.99};
    
    // should print field name and value
    refl::for_each_field(bar, [](const auto& field) {
    std::cout << field.name() << ": " << field.get() << std::endl;
    });

    return 0;
}
```