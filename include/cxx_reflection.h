#ifndef CPP_PROJECT_TEMPLATE_TEMPLATE_HEADER_H
#define CPP_PROJECT_TEMPLATE_TEMPLATE_HEADER_H

#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <variant>

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

    template<typename T, typename... FIELDS_TYPES>
    class type {
    public:
        using field_types_variant = std::variant<FIELDS_TYPES...>;

        template<typename... F>
        explicit type(F... field) {
            // and need to calculate the offset of each reflected_field
            (fields.emplace_back(std::unique_ptr<meta_field>(field.template operator()<T>())), ...);
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
    };
}

#define refl_field(NAME) []<typename T>(){ return std::make_unique<refl::reflected_field<decltype(T::NAME)>>(#NAME, offsetof(T, NAME)); }

#endif //CPP_PROJECT_TEMPLATE_TEMPLATE_HEADER_H
