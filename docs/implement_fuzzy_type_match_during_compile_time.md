# [EN] Implement Fuzzy Type Match During Compile Time

# [CN] 实现基于编译时的模糊类型匹配

## 1. 介绍

在实现基于 C++ 的反射库时，我遇到了一个新问题，如何在基于前一篇文章 [dynamic store fields type info with cpp](./dynamic_store_fields_type_info_with_cpp.md) 的前提下，可以判断传入的 `template T` 是一个 `type_placeholder`？

这一问题的解决，让我可以在反射库中更容易的声明 concept，从而保证代码的健壮。

本文将讨论我是如何基于 C++ 的模板元编程技术在编译时实现这一功能的。

## 2. 问题分析

问题其实很简单：如何判断传入的两个参数 `X` 和 `Y` 是否达成模糊匹配条件，比如：

```cpp
// 是否是一个 std::vector of something？
is_fuzzy_type_matched_v<std::vector<std::vector<int>>, std::vector<SOMETHING_I_DONT_CARE>>;

// 是否是一个 something？
is_fuzzy_type_matched_v<std::vector<int>, SOMETHING_I_DONT_CARE>;

// 某个可变参数模板是否是一个包含了 SOMETHING_I_DONT_CARE 的模板？
is_fuzzy_type_matched_v<std::tuple<int, double, std::string>, std::tuple<int, SOMETHING_I_DONT_CARE, std::string>>;

// 以此类推
```

我们可以看到，这个问题实质上要解决两个变量：
1. 深度：待断言的模板中可能嵌套着多层模板；
2. 广度：待断言的模板可能是一个可变参数模板。

## 3. 解决方案

### 3.1 从零开始

实际上，我们可以从简单着手，按照 TDD 的方式，设计并下面几个简单的测试用例:
- 当 `X` 是 `A`，且 `Y` 是 `A` 时，应返回 `true` -- 此时只需要简单封装 `std::is_same_v` 即可；
- 当 `X` 是 `A`，且 `Y` 是 `B` 时，应返回 `false` -- 不用修改代码，可直接使用 `std::is_same_v` 的逻辑；
- 当 `X` 是 `A`，且 `Y` 是 `i_dont_care` 时，应返回 `true` -- 此时需要在封装的基础上添加额外的断言了；

考虑到 C++ 中早已提供了类型萃取函数，我们可以轻易将这一能力联想到标准库函数 `std::is_same_v` 中。

因此，我们可以再次细化我们要实现的这一需求：如何扩展标准库函数 `std::is_same_v` 使其不止能够匹配完全相同的类型，还能模糊匹配类型？

此时我们有简单的代码框架如下：

```cpp
// 定义一个 i_dont_care 类作为模糊匹配标志
struct i_dont_care {};

// 定义一个模板类判断一个类型 T 是否是 i_dont_care，默认是 std::false_type
template<typename T>
struct is_i_dont_care : std::false_type {};

// 特化该模板类，当 T 是 i_dont_care 时，返回 std::true_type
template<>
struct is_i_dont_care<i_dont_care> : std::true_type {};

// 定义一个模板类，判断当两个传入类型不同时，是否有一方或者两方均存在 i_dont_care 的情况
template<typename X, typename Y>
struct is_fuzzy_type_matched {
    consteval auto operator()() {
        if constexpr (noaland::is_i_dont_care<X>::value || noaland::is_i_dont_care<Y>::value || std::is_same_v<X, Y>) {
            return std::true_type{};
        } else {
            return std::false_type{};
        }
    }
};

// 由于定义了模板的括号表达式，为了使用方便，定义一个内联常量表达式，简化调用
template<typename X, typename Y>
inline constexpr auto is_fuzzy_type_matched_v = decltype(is_fuzzy_type_matched<X, Y>()())::value;
```

### 3.2 模板递归以及元元编程

事实上，当我们实现了最简单的断言之后，我们需要考虑上面提到的两个问题，即“深度”和“广度”。鉴于这两者都是属于可变的，那么我需要做的就是使用一种递归方式，将这两个维度逐级展开成最初的简单情况。

1. 对于**深度问题**，应对的方式就是元元编程，即 `template of template` 也就是说，在 template 中，嵌套定义 template，特化 `is_fuzzy_type_matched_v` 函数，使它可以接收任何多的单个参数嵌套模板。
2. 对于**广度问题**，应对的方式是可变参数模板，既然广度问题本身面临的就是可变参数模板，那么需要做的就是定义一个可变参数模板应对这一定。

基于这两个解决办法，我们可以得到如下合并的代码框架：

```cpp
template<template<typename...> typename X, template<typename...> typename Y, typename... SUB_X, typename... SUB_Y>
struct is_fuzzy_type_matched<X<SUB_X...>, Y<SUB_Y...>> {
    consteval auto operator()() {}
};
```

接下来要做的就是对 SUB_X 和 SUB_Y 的逐级展开，鉴于之前已经定义了两个类型相同或不同的特化，接下来只需要让模板函数自动递归即可。

```cpp
// 递归并合并每个 is_fuzzy_type_matched_v 的返回值
template<bool... R>
consteval bool conjunction() {
    return (R && ...);
}

// 特化出广度和深度合并的情况
template<template<typename...> typename X, template<typename...> typename Y, typename... SUB_X, typename... SUB_Y>
struct is_fuzzy_type_matched<X<SUB_X...>, Y<SUB_Y...>> {
    consteval auto operator()() {
        if constexpr (!conjunction<is_fuzzy_type_matched_v<SUB_X, SUB_Y>...>()) {
            return std::false_type{};
        } else {
            return std::true_type{};
        }
    }
};
```

### 3.3 完整代码

最终，完整的代码如下：

```cpp
namespace noaland {
    struct i_dont_care {};

    template<typename T>
    struct is_i_dont_care : std::false_type {};

    template<>
    struct is_i_dont_care<i_dont_care> : std::true_type {};

    // if two type are not the same
    template<typename X, typename Y>
    struct is_fuzzy_type_matched {
        consteval auto operator()() {
            if constexpr (noaland::is_i_dont_care<X>::value || noaland::is_i_dont_care<Y>::value || std::is_same_v<X, Y>) {
                return std::true_type{};
            } else {
                return std::false_type{};
            }
        }
    };

    template<typename X, typename Y>
    inline constexpr auto is_fuzzy_type_matched_v = decltype(is_fuzzy_type_matched<X, Y>()())::value;

    template<bool... R>
    consteval bool conjunction() {
        return (R && ...);
    }

    template<template<typename...> typename X, template<typename...> typename Y, typename... SUB_X, typename... SUB_Y>
    struct is_fuzzy_type_matched<X<SUB_X...>, Y<SUB_Y...>> {
        consteval auto operator()() {
            if constexpr (!conjunction<is_fuzzy_type_matched_v<SUB_X, SUB_Y>...>()) {
                return std::false_type{};
            } else {
                return std::true_type{};
            }
        }
    };
}
```

你可以直接在 `noaland_lib.h` 中找到该函数，并直接使用 `noaland::is_fuzzy_type_matched_v<X, Y>` 进行调用。

同时，由于该函数所依赖的所有函数都是 `consteval` 的，因此你可以在编译时直接调用该函数，而不需要在运行时调用。也就是说，该函数可以直接 `static_assert` 进行断言，而不会占用任何运行时资源。

## 4. 总结

尽管这篇文章看起来只是解决了一个简单的问题，但其实涉及到了很多 C++ 模板元编程的实践，如模板递归、元元编程、可变参数模板、模板特化、`consteval` 等等。

这些技巧充满了惊喜，当然也充满了惊吓，写反射库的过程中，我不停的在“太好了，这条路通了”和“完蛋了，这条路断了”之间反复横跳。

如果你对本文阐述的内容感兴趣，请查看我正在**手搓过程中**的 [cxx-reflection](https://github.com/NoaLand/cxx-reflection) 代码库，欢迎 star 和 fork。