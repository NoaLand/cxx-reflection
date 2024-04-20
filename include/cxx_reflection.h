#ifndef CPP_PROJECT_TEMPLATE_TEMPLATE_HEADER_H
#define CPP_PROJECT_TEMPLATE_TEMPLATE_HEADER_H

#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <variant>
#include <array>
#include <iostream>

namespace refl {
    struct meta_field {
        virtual std::string get_name() = 0;
        virtual size_t get_offset() = 0;
        virtual ~meta_field() = default;
    };

    template<typename FT>
    class reflected_field : public meta_field {
    public:
        reflected_field(std::string name, const size_t& offset) : name{std::move(name)}, offset{offset} {}
        std::string get_name() override { return name; }
        size_t get_offset() override { return offset; }
    private:
        std::string name;
        size_t offset;
    };

    template<typename T>
    struct type_identity {
        using type = T;
    };

    template<typename T>
    consteval type_identity<T> create_type_identity() {
        return {};
    }

    template<typename T>
    consteval T extract_field_type(std::unique_ptr<refl::reflected_field<T>>);

    template<typename T, typename... FIELDS_TYPES>
    class type {
    public:
        // TODO: handle duplicated types later, since std::variant cannot hold duplicated types
        using field_types_variant = std::variant<type_identity<FIELDS_TYPES>...>;

        template<typename... F>
        explicit type(F... field) {
            // and need to calculate the offset of each reflected_field
            (fields.emplace_back(std::unique_ptr<meta_field>(field.template operator()<T>())), ...);
        }

        template<std::size_t INDEX>
        consteval auto get_field_type(const field_types_variant& some_field) {
            return static_cast<typename std::decay_t<decltype(std::get<INDEX>(some_field))>::type*>(nullptr);
        }

        // TODO: should be changed later according
        template<typename FT>
        void set_field_value(T& instance, const std::string& field_name, FT&& value) {
            for (const auto& f : fields) {
                if (f->get_name() == field_name) {
                    *reinterpret_cast<FT*>(reinterpret_cast<char*>(&instance) + f->get_offset()) = value;
                    return;
                }
            }
        }

    // private:
        std::vector<std::unique_ptr<meta_field>> fields;
        static constexpr std::array<std::pair<std::string_view, field_types_variant>, sizeof...(FIELDS_TYPES)> fields_x;
    };
}

#define refl_field(NAME) []<typename T>(){ return std::make_unique<refl::reflected_field<decltype(T::NAME)>>(#NAME, offsetof(T, NAME)); }

#endif //CPP_PROJECT_TEMPLATE_TEMPLATE_HEADER_H
