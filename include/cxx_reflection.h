#ifndef CPP_PROJECT_TEMPLATE_TEMPLATE_HEADER_H
#define CPP_PROJECT_TEMPLATE_TEMPLATE_HEADER_H

#include <string>
#include <memory>
#include <utility>
#include <vector>

namespace refl {
//    namespace {
        struct field_base {
            virtual std::string get_name() = 0;
            virtual size_t get_offset() = 0;
            virtual ~field_base() = default;
        };

        template<typename FT>
        class _field : public field_base {
        public:
            _field(std::string name, const size_t& offset) : name{std::move(name)}, offset{offset} {}
            std::string get_name() override { return name; }
            size_t get_offset() override { return offset; }
        private:
            std::string name;
            size_t offset;
            using type = FT;
        };
//    }

    template<typename T>
    class type {
    public:
        template<typename... F>
        explicit type(F... field) {
            // and need to calculate the offset of each field
            (fields.emplace_back(std::unique_ptr<field_base>(field.template operator()<T>())), ...);
        }

    // private:
        std::vector<std::unique_ptr<field_base>> fields;
    };
}

#define refl_field(NAME) []<typename T>(){ return std::make_unique<refl::_field<decltype(T::NAME)>>(#NAME, offsetof(T, NAME)); }

#endif //CPP_PROJECT_TEMPLATE_TEMPLATE_HEADER_H
