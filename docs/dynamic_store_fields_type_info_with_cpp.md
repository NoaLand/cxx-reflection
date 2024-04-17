[TOC]

# [EN] Dynamic Store Fields Type Info with C++

## 1. Introduction

One important problem that a C++ reflection library aims to address is how to "dynamically" retrieve type information from registered reflected types.

Since C++ itself is a statically-typed system, we need a mechanism to generate all the relevant information related to fields during compile-time, such as "field name," "offset relative to its enclosing type," and "field type information."

Note: The term "field type information" refers to the type information of the field, not its value. For example, for a field `int a`, its type information is `int`, not `18`. In other words, in the subsequent implementation, we expect to create a variable of the same type as `int` using this information, as shown below:

```cpp
// pseudocode
struct Foo {
    int a;
};

refl(type(Foo), field(a)) refl_foo;

TEST(refl_test, should_get_type_info_after_register) {
    refl_foo.get_type("a")::type x = 10;

    ASSERT_TRUE(std::is_same_v<decltype(x), int>);
}
```

## 2. Problem Definition

In a reflection library, potential use cases may include:
- When implementing an "auto fixture" testing framework (a framework used for quickly creating test data), the ability to retrieve the type information of fields based on their names is required. This information is then used to generate a value of the same type and assign it to the generated instance.
- When implementing serialization/deserialization, the ability to retrieve the type information of fields based on their names is necessary. This information is then used to generate a value of the same type and assign it to the generated instance.
- When implementing assignment validation in a reflection library, it is necessary to determine whether the type of field matches and if it is assignable.

To achieve this capability, we need to address the following problems:
- Constraint 1: C++ is a statically-typed system, meaning that dynamic types cannot be determined or generated at runtime. Therefore, to generate type information at compile-time, we need to employ a mechanism that generates all the relevant information related to fields, such as "field name," "offset relative to its enclosing type," and "field type information."
- Constraint 2: Although it is possible to retrieve type information using `typeid(T).name()` or `typeid(T).hashcode()`, this information cannot be used to reconstruct types.
- Constraint 3: Considering the aforementioned requirements, we need a solution that allows us to store both the type information and all related information in a single container for easy querying or traversal.
  - It is worth noting that this aspect will be a crucial consideration in designing this functionality because certain features in C++, such as template specialization, inheritance, and `std::any`, allow for storage in a container but result in the loss of type information (as mentioned later in the text).

## 3. Solution Analysis

### 3.1 [Failed] Solution I: std::any

Using `std::any` to store information has its drawbacks. It is evident that when information is stored in an `any` object, the original information is completely decayed to the `std::any` type. As a result, it becomes impossible to retrieve any meaningful information. To obtain the information, one needs to convert the `any` object to a specific type. However, at runtime, the code loses its type information, making it impossible to perform successful conversions, let alone accessing the information stored in the `any` object.

### 3.2 [Failed] Solution II: Template Class + Inheritance

⚠️ Note: This article does not discuss the calculation of offsets. Content related to offsets will be addressed in a separate article.

To store different types in the same container, the use of inheritance seems to be a suitable approach. For this purpose, I conducted the following experiment:

```cpp
#include "gtest/gtest.h"

#include <string>
#include <vector>
#include <memory>

struct field_base {
    virtual const std::string get_name() = 0;
    virtual std::string get_type_info() = 0;
    virtual ~field_base() = default;
};

template<typename T>
struct field : field_base {
    field(const std::string& name) : name(name) {}
    
    virtual const std::string get_name() override {
        return name;
    }

    virtual std::string get_type_info() override {
        return typeid(T).name();
    }
    
    using type = T;
    
private:
    std::string name;
};

TEST(refl_test, should_get_type_info_after_register) {
    std::vector<std::unique_ptr<field_base>> fields;
    fields.push_back(std::make_unique<field<int>>("int_field"));
    fields.push_back(std::make_unique<field<double>>("double_field"));    
    
    ASSERT_EQ(fields[0]->get_name(), "int_field");
    ASSERT_EQ(fields[0]->get_type_info(), "i");
    
    ASSERT_EQ(fields[1]->get_name(), "double_field");
    ASSERT_EQ(fields[1]->get_type_info(), "d");
}
```

However, this approach still has limitations. Although I can store the type `T` as the alias `type`, when using `std::vector<field_base>`, I cannot directly use this alias and instead have to rely on the encapsulated virtual function `virtual std::string get_type_info()`.

As a result, I can retrieve the type name of the field, such as `"i"` or `"d"` in the example above, but I cannot generate a variable of type `int` or `double` based solely on this information.

Similarly, I cannot define a template virtual function or a dynamic type alias in the base class `field_base`.

Thus, this solution also fails to meet the requirements.

### 3.3 [Successful] Solution III: constexpr + Template Specialization/Partial Specialization + Template Recursion

To address the aforementioned issues, one approach is as follows:
1. Define a template class with a template type of `int` to tackle the problem of storing information in the same container. The elements within this container will be of type `field<int>()`, where `int` represents the index of a custom type.
2. [Using Macros] When registering type information, construct a template specialization for `template<int index> struct field {};`. For example, in the current type system, we define the index of `int` as `0` and the index of `double` as `1`, and so on.

However, this solution has drawbacks:
1. It heavily relies on macros, making the code difficult to maintain.
2. Due to time constraints during implementation, the solution is not perfect, lacks generality, and appears unrefined.

Therefore, this solution is not an ideal one, and due to certain factors, it is not presented here.

### 3.4 [Successful] Solution IV: constexpr + std::variant + Variadic Template Class (Used in Reflection Library)

⚠️ This solution is also used in this codebase.

In this solution, several corresponding measures are taken to address the constraints mentioned earlier:
1. For Constraint 1:
    1. Use `constexpr` to generate type information at compile-time, such as field names and field type information. This allows the reflection library to "appear" as if it dynamically registers information, while in reality, all this information is already registered and calculated during compilation.
    2. To perform computations at compile-time, all calculation functions need to be defined as `consteval` and `constexpr`. Therefore, `std::array<std::pair<...>, ARRAY_SIZE>` is used as a table instead of `std::map` or `std::vector`.
2. For Constraint 2:
    1. Use `type_placeholder` as the storage structure for type information, directly storing types instead of type information, enabling retrieval of types at runtime.
    2. Construct the template function `template<std::size_t index> consteval auto get_type_info()` to leverage the compile-time computation capability of `consteval`, the dynamic determination of return types using `auto`, and the conditional compilation ability of `if constexpr`. This implementation makes the `get_type` function "appear" as if it returns dynamic types.
3. For Constraint 3, since the type information in the reflection library is registered first and then used, we can ascertain that the fields in the class are known. Thus, we can use `std::variant` combined with a variadic template class to construct a `std::variant` that holds all the type information existing in a class.

⚠️ Note: Additionally, considerations should be given to scenarios without default constructors and memory efficiency. Therefore, the structure for storing type information should not store instances but only types.

To achieve this, I made the first version attempt, which successfully stores custom type information. However, due to the use of `*static_cast<T>(nullptr)`, it produces errors for primitive data types. This issue is completely resolved in the second version:

```cpp
#include "gtest/gtest.h"

#include <string>
#include <variant>
#include <functional>
#include <iostream>
#include <vector>

// v1
// Assuming there is a class without a default constructor
struct Foo {
    Foo() = delete;
};

// When called externally, it cannot handle primitive data types like int, as there will be problems when dereferencing.
template<std::size_t index>
consteval auto get_type(const std::variant<int, std::string, Foo>& my_variant) {
    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, int>) {
        return static_cast<int*>(nullptr);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, std::string>) {
        return static_cast<std::string*>(nullptr);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, Foo>) {
        return static_cast<Foo*>(nullptr);
    }
}

// This code compiles successfully under clang trunk, but encounters issues with gcc
TEST(refl_test, should_get_type_info_after_register) {
    using field_types = std::variant<int, std::string, Foo>;
    constexpr field_types my_variant = *static_cast<Foo*>(nullptr);
    ASSERT_TRUE(std::is_same_v<std::remove_pointer<decltype(get_type<my_variant.index()>(my_variant))>::type, Foo>);
}
```

The v1 version of the code has a few problems:
1. It cannot handle primitive data types as there are compilation errors when dereferencing.
2. It cannot compile successfully under gcc.

Therefore, in the improved v2 version, I used a custom `type_placeholder` as the type management structure and returned a type assumed using nullptr in the `get_type` function to obtain type information when used.

```cpp
#include "gtest/gtest.h"

#include <string>
#include <variant>
#include <iostream>
#include <vector>

struct Foo {
    Foo() = delete;
};

// v2
// type_placeholder is used to store field type information
template<typename T>
struct type_placeholder {
    using type = T;
};

// Used to create instances of type_placeholder
template<typename T>
consteval type_placeholder<T> create_tp() {
    return {};
}

// To simplify the code, field_types is used as an alias for all field types
using field_types = std::variant<type_placeholder<int>, type_placeholder<std::string>, type_placeholder<Foo>, type_placeholder<int*>>;

// Get pointers to the field types here to avoid dereferencing issues. Additionally, consteval is used here, which allows for compile-time computation. In practical usage, variadic template expansion can be used to implement the ability of if constexpr.
template<std::size_t index>
consteval auto get_type(const field_types& my_variant) {
    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, type_placeholder<int>>) {
        return static_cast<int*>(nullptr);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, type_placeholder<int*>>) {
        return static_cast<int**>(nullptr);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, type_placeholder<std::string>>) {
        return static_cast<std::string*>(nullptr);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, type_placeholder<Foo>>) {
        return static_cast<Foo*>(nullptr);
    }
}

TEST(refl_test, should_get_type_info_after_register) {
    // register 4 fields: int i, Foo foo, std::string s, int* Pi
    constexpr std::array<std::pair<std::string_view, field_types>, 4> x {{
        {std::string_view{"i"}, create_tp<int>()},
        {std::string_view{"foo"}, create_tp<Foo>()},
        {std::string_view{"s"}, create_tp<std::string>()},
        {std::string_view{"Pi"}, create_tp<int*>()},
    }};
    
    // Type information holder check. Next, determine whether it is possible to retrieve the type information for each field from the array, and use std::is_same_v to verify its correctness.
    constexpr field_types i = x[0].second;
    ASSERT_TRUE((std::is_same_v<std::remove_pointer<decltype(get_type<i.index()>(i))>::type, int>));

    constexpr field_types foo = x[1].second;
    ASSERT_TRUE((std::is_same_v<std::remove_pointer<decltype(get_type<foo.index()>(foo))>::type, Foo>));

    constexpr field_types d = x[2].second;
    ASSERT_TRUE((std::is_same_v<std::remove_pointer<decltype(get_type<d.index()>(d))>::type, std::string>));

    constexpr field_types Pi = x[3].second;
    ASSERT_TRUE((std::is_same_v<std::remove_pointer<decltype(get_type<Pi.index()>(Pi))>::type, int*>));
}
```

By doing so, we have successfully used `std::variant` and `type_placeholder<T>` to store the actual type information, and we can retrieve the type information based on the `index`. Now, all that's left is to integrate the `std::variant` into the variadic template class of `refl::type` in the reflection library.

## 4. Conclusion

In conclusion, as mentioned at the beginning of the article, due to C++'s static type system, all we can do is simulate dynamic behavior for type registration and type information retrieval at runtime through compile-time computation. However, upon careful analysis of the implementations of "Solution III" and "Solution IV," we discover that all the information required at runtime is actually computed during compilation. Therefore, there is no real "dynamic type retrieval," as everything is directly obtained.

To address the issues raised in this article, what I have done is simply to seek the "invariant" amidst the "changes," find the "static" within the "dynamic," and explore the "compile-time" within the "runtime" in order to discover what seems "impossible" but is actually "possible." This is the charm of reinventing the wheel in C++.

If you are interested in the content presented in this article, please take a look at my **work-in-progress** [cxx-reflection](https://github.com/NoaLand/cxx-reflection) repository on GitHub. Your stars and forks are welcome.

---

# [CN] 在 C++ 静态类型系统中实现动态类型信息存储

## 1. 介绍

基于 C++ 的反射库要解决的一个重要问题就是：如何能够在注册反射类型之后，“看起来”是动态的获取字段中的类型信息。

由于 C++ 本身是静态类型系统，因此，我们需要使用一种手段，即在编译时，生成一切字段相关的信息，如：“字段名称 name”、“字段相对其所在类型的偏移量 offset”、“字段类型信息”。

注意：这里的“字段类型信息”是指字段的类型信息，而不是字段的值。例如，对于一个字段 `int a`，其类型信息是 `int`，而不是 `18`。也就是说，在后续的实现中，我们期望可以通过 `int` 这个信息创建一个类型同样为 `int`  的变量，形如：

```cpp
// pseudocode
struct Foo {
    int a;
};

refl(type(Foo), field(a)) refl_foo;

TEST(refl_test, should_get_type_info_after_register) {
    refl_foo.get_type("a")::type x = 10;

    ASSERT_TRUE(std::is_same_v<decltype(x), int>);
}
```

## 2. 问题定义

在反射库中，可能存在的使用场景如：
- 实现测试框架 `auto fixture` 时（一种用于快速创建测试数据的测试框架），需要能够根据字段名称获取字段的类型信息，并根据该类型信息生成一个同样类型的值并赋值给生成的实例；
- 实现序列化/反序列化，需要能够根据字段名称获取字段的类型信息，并根据该类型信息生成一个同样类型的值并赋值给生成的实例；
- 实现反射库中赋值校验，需要判断字段的类型是否匹配，以及是否可以赋值；

为了能够实现这一能力，我们需要解决以下问题：
- 约束1: C++ 是静态类型系统，即无法在运行时确定或生成动态类型，因此为了能够在编译时生成类型信息，我们需要使用一种手段，即在编译时，生成一切字段相关的信息，如：“字段名称 name”、“字段相对其所在类型的偏移量 offset”、“字段类型信息”；；
- 约束2: 尽管能够通过 typeid(T).name() 或 typeid(T).hashcode() 获取类型信息，但这种信息无法反向构建成类型；
- 约束3: 鉴于上述的需求，我们需要：既保存类型信息，又能够将所有的信息保存在同一个容器中，以便后续查询或遍历；
  - 值得一提的是，这一点会成为设计该功能时重要的考验，因为 C++ 中如模板特化、继承、std::any 尽管能够统一存储在容器中，但同样的也会失去这些类型信息（后文会提到）；

## 3. 解决方案分析

### 3.1 [失败] 方案一：std::any

使用 any 来存放信息的弊端，但事实显而易见，使用 any 存放信息时，会将原本的信息**完全退化**为 any 类型，以至于完全无法获取到有效信息，因为为了能够获取信息我需要将 any 转换为某种类型，但在运行时，代码失去了类型信息，因此无法成功转换，更别提获取 any 中的信息了。

### 3.2 [失败] 方案二：模板类 + 继承

⚠️注意：本文不讨论 offset 的计算，有关 offset 相关内容会另外单独写一篇文章。

为了在同一容器中存放不同的类型，这一需求显然使用继承可以完全符合，为此我进行了下列实验：

```cpp
#include "gtest/gtest.h"

#include <string>
#include <vector>
#include <memory>

struct field_base {
    virtual const std::string get_name() = 0;
    virtual std::string get_type_info() = 0;
    virtual ~field_base() = default;
};

template<typename T>
struct field : field_base {
    field(const std::string& name) : name(name) {}
    
    virtual const std::string get_name() override {
        return name;
    }

    virtual std::string get_type_info() override {
        return typeid(T).name();
    }
    
    using type = T;
    
private:
    std::string name;
};

TEST(refl_test, should_get_type_info_after_register) {
    std::vector<std::unique_ptr<field_base>> fields;
    fields.push_back(std::make_unique<field<int>>("int_field"));
    fields.push_back(std::make_unique<field<double>>("double_field"));    
    
    ASSERT_EQ(fields[0]->get_name(), "int_field");
    ASSERT_EQ(fields[0]->get_type_info(), "i");
    
    ASSERT_EQ(fields[1]->get_name(), "double_field");
    ASSERT_EQ(fields[1]->get_type_info(), "d");
}
```

但这样做仍然存在问题，即，我能够存储类型 `T` 为别名 `type` 但在使用 `std::vector<field_base>` 时，却不能直接用该别名，只能使用封装的虚函数 `virtual std::string get_type_info()`。

这样一来，我能够知道该字段的类型名字，如上例中的 `"i"`、`"d"`，但不能根据这个信息生成一个为 `int` 或 `double` 的变量。

同样的，我也无法在基类 `field_base` 中定义一个模板虚函数，或是一个动态的类型别名。

到此，该同样方案失败。

### 3.3 [成功] 方案三：constexpr + 模板特化/偏特化 + 模板递归

为了解决上述问题，一种解决思路是：
1. 定义一个模板类型为 int 的模板类，用于解决如何在同一容器中存储信息的问题：因为其中的元素类型均为 `field<int>()` 其中 `int` 代表的是自定义类型的索引。
2. [使用宏] 使用宏注册类型信息时，构造一个对于 `template<int index> struct field {};` 的模板特化，如当前类型系统中，我将 `int` 的索引定义为 `0`，`double` 的索引定义为 `1`，以此类推。

但该方案由于：
1. 大量使用宏，使得代码难以维护；
2. 实现时由于时间有限，因此实现并不完善也很丑陋，且不够通用；

因此，该方案不是一个很好的解决方案，加上由于某些因素，在此不展示。

### 3.4 [成功] 方案四：constexpr + std::variant + 可变参数模板类（集成到反射库时需要用到）

⚠️该方案也是本代码库使用的方案。

本方案中，为了解决前面提到的几个约束，分别做了对应的应对措施：
1. 对于约束1：
   1. 使用 `constexpr` 在编译时生成类型信息，如字段名称、字段类型信息等，让反射库注册时“看起来”是动态注册的一样，其实在编译时这些信息就都已经被注册成功并计算出来了；
   2. 为了能够在编译时执行计算，需要将所有计算函数定义为 `consteval` 和 `constexpr`，因此，需要使用 `std::array<std::pair<...>, ARRAY_SIZE>` 作为表，而非 `std::map` 或 `std::vector`；
2. 对于约束2：
   1. 使用 type_placeholder 作为类型信息的保存结构，直接保存类型，而非类型信息，以便能够在运行时获取类型；
   2. 构造模板函数 `template<std::size_t index> consteval auto get_type_info()`，借助 `consteval` 的编译时计算能力，以及 `auto` 可动态决定返回类型，以及 `if constexpr` 能力，将 `get_type` 函数实现为“看起来”返回动态类型的函数；
3. 对于约束3，由于反射库中的类型信息会先注册，再使用，因此我们可以确认类中的字段一定是已知的，“字段数量”，“字段类型”，“字段名称”等，由此可以使用 `std::variant` 结合 可变参数模板类构造 `std::variant` 用于存放一个类中存在的所有类型信息；

⚠️注意：另外还需要考虑无默认构造函数与内存效率的情况，因此对于类型信息保存的结构体，不能保存成实例，而只应保存类型。

为此，我做了第一个版本的尝试，该版本能成功保存自定义类型信息，但由于使用了 `*static_cast<T>(nullptr)`，因此对于基本数据类型会出现报错，这一问题在第二版中完全解决：

```cpp
#include "gtest/gtest.h"

#include <string>
#include <variant>
#include <functional>
#include <iostream>
#include <vector>

// v1
// 假设有一个没有默认构造函数的类
struct Foo {
    Foo() = delete;
};

// 外层调用时，无法处理 int 这种基本数据类型，因为在解引用时会遇到问题。
template<std::size_t index>
consteval auto get_type(const std::variant<int, std::string, Foo>& my_variant) {
    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, int>) {
        return static_cast<int*>(nullptr);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, std::string>) {
        return static_cast<std::string*>(nullptr);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, Foo>) {
        return static_cast<Foo*>(nullptr);
    }
}

// 本代码在 clang trunk 下可以编译通过，在 gcc 会遇到问题
TEST(refl_test, should_get_type_info_after_register) {
    using field_types = std::variant<int, std::string, Foo>;
    constexpr field_types my_variant = *static_cast<Foo*>(nullptr);
    ASSERT_TRUE(std::is_same_v<std::remove_pointer<decltype(get_type<my_variant.index()>(my_variant))>::type, Foo>);
}
```

v1 版本的代码有几个问题：
1. 无法处理基本数据类型，因为解引用时会编译报错；
2. 在 gcc 下无法通过编译；

为此，改进版的 v2，使用了自定义的 `type_placeholder` 作为类型的管理结构，使用 `get_type` 函数返回一个使用 nullptr 假装的类型，以便在使用时获取类型信息。

```cpp
#include "gtest/gtest.h"

#include <string>
#include <variant>
#include <iostream>
#include <vector>

struct Foo {
    Foo() = delete;
};

// v2
// type_placeholder 用于保存字段类型信息
template<typename T>
struct type_placeholder {
    using type = T;
};

// 用于创建 type_placeholder 实例
template<typename T>
consteval type_placeholder<T> create_tp() {
    return {};
}

// 为了简化代码在此设置全部字段类型的别名 field_types
using field_types = std::variant<type_placeholder<int>, type_placeholder<std::string>, type_placeholder<Foo>, type_placeholder<int*>>;

// 在此获取字段类型的指针，避免解引用的问题，另外，这里使用了 consteval，因此可以在编译时计算，在实际使用中，可以使用可变参数展开实现 `if constexpr` 的能力
template<std::size_t index>
consteval auto get_type(const field_types& my_variant) {
    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, type_placeholder<int>>) {
        return static_cast<int*>(nullptr);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, type_placeholder<int*>>) {
        return static_cast<int**>(nullptr);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, type_placeholder<std::string>>) {
        return static_cast<std::string*>(nullptr);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(std::get<index>(my_variant))>, type_placeholder<Foo>>) {
        return static_cast<Foo*>(nullptr);
    }
}

TEST(refl_test, should_get_type_info_after_register) {
    // 注册四个字段，分别为 int i, Foo foo, std::string s, int* Pi
    constexpr std::array<std::pair<std::string_view, field_types>, 4> x {{
        {std::string_view{"i"}, create_tp<int>()},
        {std::string_view{"foo"}, create_tp<Foo>()},
        {std::string_view{"s"}, create_tp<std::string>()},
        {std::string_view{"Pi"}, create_tp<int*>()},
    }};
    
    // 类型信息持有判断，接下来判断是否能够从 array 中恢复每个字段的类型信息，并使用 std::is_same_v 判断是否正确
    constexpr field_types i = x[0].second;
    ASSERT_TRUE((std::is_same_v<std::remove_pointer<decltype(get_type<i.index()>(i))>::type, int>));

    constexpr field_types foo = x[1].second;
    ASSERT_TRUE((std::is_same_v<std::remove_pointer<decltype(get_type<foo.index()>(foo))>::type, Foo>));

    constexpr field_types d = x[2].second;
    ASSERT_TRUE((std::is_same_v<std::remove_pointer<decltype(get_type<d.index()>(d))>::type, std::string>));

    constexpr field_types Pi = x[3].second;
    ASSERT_TRUE((std::is_same_v<std::remove_pointer<decltype(get_type<Pi.index()>(Pi))>::type, int*>));
}
```

这样一来，就成功使用 `std::variant` 和 `type_placeholder<T>` 保存了事实类型信息，并可以根据 `index` 获取到类型信息。接下来只需要将 `std::variant` 集成反射库中的 `refl::type` 的可变参数模板类即可。

## 4. 总结

综上所述，正如文章开头所述，由于 C++ 的静态类型系统，我们能做的，只是通过编译时计算，让类型注册、获取类型信息在运行时**“看起来像是动态”**完成的。但实际上，仔细分析“方案三”和“方案四”的实现，我们会发现，所有运行时所需的信息，其实在编译期就已经被计算出来了，因此并不存在真正的“动态类型获取”，一切都是直接获取而已。

为了解决本文提出的问题，我所做的其实只是：在“变化”中寻找“不变”，在“动态”中寻找“静态”，在“运行时”中寻找“编译时”，才能在“不可能”中寻找“可能”。这也是在 C++ 里造轮子的魅力所在。

如果你对本文阐述的内容感兴趣，请查看我正在**手搓过程中**的 [cxx-reflection](https://github.com/NoaLand/cxx-reflection) 代码库，欢迎 star 和 fork。