#ifndef __CXX_REFLECTION_NOALAND_LIB_H__
#define __CXX_REFLECTION_NOALAND_LIB_H__

#include <type_traits>

namespace noaland {
    struct i_dont_care {};

    // if two type are not the same
    template<typename X, typename Y>
    struct is_a{
        consteval auto operator()() {
            if constexpr (std::is_same_v<X, i_dont_care> || std::is_same_v<Y, i_dont_care>) {
                return std::true_type{};
            } else {
                return std::false_type{};
            }
        }
    };

    // if two type are the same
    template<typename X>
    struct is_a<X, X> {
        consteval auto operator()() {
            return std::true_type{};
        }
    };

    template<typename X, typename Y>
    inline constexpr auto is_a_v = decltype(is_a<X, Y>()())::value;
}

#endif // __CXX_REFLECTION_NOALAND_LIB_H__
