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
        expected(V val) { std::construct_at(&val_, std::move(val)); t_ = expected_type::VALUE; }
        expected(unexpected<E> err) { std::construct_at(&err_, std::move(err)); t_ = expected_type::ERROR; }
        ~expected() {
            if (t_ == expected_type::VALUE) {
                val_.~V();
            } else {
                err_.~unexpected<E>();
            }
            t_ = expected_type::ERROR;
        }

        expected and_should(auto validation_func, E err) {
            switch (t_) {
                case expected_type::VALUE:
                    if (validation_func(val_)) {
                        return { val_ };
                    } else {
                        return unexpected<E>(std::move(err));
                    }
                case expected_type::ERROR:
                default:
                    return unexpected<E>(std::move(err));
            }
        }

        expected and_then(auto operation_func) {
            switch (t_) {
                case expected_type::VALUE:
                    return operation_func(std::move(val_));
                case expected_type::ERROR:
                default:
                    return unexpected<E>(std::move(err_.what()));
            }
        }

        expected or_else(V default_val, auto throw_func) {
            switch (t_) {
                case expected_type::VALUE:
                    return val_;
                case expected_type::ERROR:
                default:
                    throw_func(err_);
                    return default_val;
            }
        }

        auto value() {
            return val_;
        }

    private:
        enum class expected_type { VALUE = 0, ERROR };
        union {
            V val_;
            unexpected<E> err_;
        };
        expected_type t_{ expected_type::ERROR };

    };
}

#endif // __CXX_REFLECTION_NOALAND_LIB_H__
