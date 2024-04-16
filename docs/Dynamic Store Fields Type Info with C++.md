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

void using_refl() {
    get_type<Foo>("a")::type x = 10;
    
    static_assert(std::is_same_v<decltype(x), int>);
}
```