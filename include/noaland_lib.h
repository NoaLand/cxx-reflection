#ifndef __CXX_REFLECTION_NOALAND_LIB_H__
#define __CXX_REFLECTION_NOALAND_LIB_H__

#include <type_traits>

namespace noaland {
    struct i_dont_care {};

    template<typename T>
    struct is_i_dont_care : std::false_type {};

    template<>
    struct is_i_dont_care<i_dont_care> : std::true_type {};

    // if two type are not the same
    template<typename X, typename Y>
    struct is_a {
        consteval auto operator()() {
            if constexpr (noaland::is_i_dont_care<X>::value || noaland::is_i_dont_care<Y>::value || std::is_same_v<X, Y>) {
                return std::true_type{};
            } else {
                return std::false_type{};
            }
        }
    };

    template<typename X, typename Y>
    inline constexpr auto is_a_v = decltype(is_a<X, Y>()())::value;

    template<bool... R>
    consteval bool conjunction() {
        return (R && ...);
    }

    template<template<typename...> typename X, template<typename...> typename Y, typename... SUB_X, typename... SUB_Y>
    struct is_a<X<SUB_X...>, Y<SUB_Y...>> {
        consteval auto operator()() {
            if constexpr (!conjunction<is_a_v<SUB_X, SUB_Y>...>()) {
                return std::false_type{};
            } else {
                return std::true_type{};
            }
        }
    };
}

#endif // __CXX_REFLECTION_NOALAND_LIB_H__
