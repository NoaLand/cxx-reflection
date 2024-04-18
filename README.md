# Open CXX Reflection

This is a C++ reflection library that is designed to be easy to use and is written in C++20.

And I'm trying to make it as simple as possible, so that user can use it without changing their implementation code.

BTW, in this library, instead of using macro, this lib will prefer to use template to do as many reflection features as possible.

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
// TBD, maybe change later
refl::type<Foo> foo{refl::reflected_field<int>("i"), refl::reflected_field<double>("d")};
refl::type<Bar> bar{refl::reflected_field<Foo>("foo"), refl::reflected_field<std::string>("str")};

// use them in function
int main() {
    Bar bar{{1, 3.14}, "hello"};
    
    // should get reflected_field value by name
    auto str1 = refl::get_field<bar>("str");
    
    // should use type of reflected_field as type of variable,
    // create a new variable named "str2" with type std::string
    refl::get_field<bar>("str") str2{"world"};
    
    // create a new variable named "foo" with type Foo
    refl::get_field<bar>("foo") foo{2, 7.99};
    
    // should print reflected_field name and value
    refl::for_each_field<bar>([](const auto& reflected_field) {
        std::cout << reflected_field.name() << ": " << reflected_field.get() << std::endl;
    });

    return 0;
}
```