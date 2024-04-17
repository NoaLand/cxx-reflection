# Dynamic Store Fields Type Info with C++

## English Version

## 中文版

### 1. 介绍

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
    get_type<Foo>("a")::type x = 10;

    ASSERT_TRUE(std::is_same_v<decltype(x), int>);
}
```

### 2. 问题定义

在反射库中，可能存在的使用场景如：

- 实现测试框架 `auto fixture` 时（一种用于快速创建测试数据的测试框架），需要能够根据字段名称获取字段的类型信息，并根据该类型信息生成一个同样类型的值并赋值给生成的实例；
- 实现序列化/反序列化，需要能够根据字段名称获取字段的类型信息，并根据该类型信息生成一个同样类型的值并赋值给生成的实例；
- 实现反射库中赋值校验，需要判断字段的类型是否匹配，以及是否可以赋值；

为了能够实现这一能力，我们需要解决以下问题：

- 约束1: C++ 是静态类型系统，即无法在运行时确定或生成动态类型，因此为了能够在编译时生成类型信息，我们需要使用一种手段，即在编译时，生成一切字段相关的信息，如：“字段名称 name”、“字段相对其所在类型的偏移量 offset”、“字段类型信息”；；
- 约束2: 尽管能够通过 typeid(T).name() 或 typeid(T).hashcode() 获取类型信息，但这种信息无法反向构建成类型；
- 约束3: 鉴于上述的需求，我们需要：既保存类型信息，又能够将所有的信息保存在同一个容器中，以便后续查询或遍历；
  - 值得一提的是，这一点会成为设计该功能时重要的考验，因为 C++ 中如模板特化、继承、std::any 尽管能够统一存储在容器中，但同样的也会失去这些类型信息（后文会提到）；

### 3. 解决方案分析

#### 3.1 【失败】方案一：std::any

使用 any 来存放信息的弊端，但事实显而易见，使用 any 存放信息时，会将原本的信息**完全退化**为 any 类型，以至于完全无法获取到有效信息，因为为了能够获取信息我需要将 any 转换为某种类型，但在运行时，代码失去了类型信息，因此无法成功转换，更别提获取 any 中的信息了。

#### 3.2 【失败】方案二：模板类 + 继承

⚠️注意：本文不讨论 offset 的计算，有关 offset 相关内容会另外单独写一篇文章。

为了在同一容器中存放不同的类型，这一需求显然使用继承可以完全符合，为此我进行了下列实验：

```cpp
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