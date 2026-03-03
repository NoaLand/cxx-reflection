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

    // expected
    template<typename T>
    concept printable = requires(T t) {
        { std::cout << t } -> std::same_as<std::ostream&>;
    };

    template<printable E>
    class unexpected {
    public:
        explicit unexpected(E ex) : err{std::move(ex)} {}
        E what() const { return err; }

    private:
        E err;
    };

    template<class... Ts> struct overloads : Ts... { using Ts::operator()...; };
    template<class... Ts> overloads(Ts...) -> overloads<Ts...>;

    template<typename V, typename E>
    class expected {
    public:
        expected(V val) : inner_{std::move(val)} {}
        expected(unexpected<E> err) : inner_{std::move(err)} {}

        expected and_should(auto validation_func, E err) {
            return std::visit(overloads {
                [validation_func, err](V& val) -> expected {
                    if (validation_func(val)) {
                        return std::move(val);
                    } else {
                        return unexpected<E>(std::move(err));
                    }
                },
                [](auto&& err) -> expected {
                    return unexpected<E>(std::move(err.what()));
                }
            }, inner_);
        }

        expected and_then(auto operation_func) {
            return std::visit(overloads {
                [operation_func](V& val) -> expected {
                    return operation_func(std::move(val));
                },
                [](auto&& err) -> expected {
                    return unexpected<E>(std::move(err.what()));
                }
            }, inner_);
        }

        expected or_else(auto throw_func) {
            return std::visit(overloads {
                [](V& val) -> expected {
                    return std::move(val);
                },
                [throw_func](auto&& err) -> expected {
                    throw_func(err);
                    return unexpected<E>(std::move(err.what()));
                }
            }, inner_);
        }

        V value_or(V&& default_val) {
            return std::visit(overloads {
                [](V& val) -> V {
                    return std::move(val);
                },
                [&default_val](auto&&) -> V {
                    return std::move(default_val);
                }
            }, inner_);
        }

    private:
        std::variant<V, unexpected<E>> inner_;
    };
}

#endif // __CXX_REFLECTION_NOALAND_LIB_H__
