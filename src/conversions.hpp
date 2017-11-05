#include <memory>
#include <unordered_map>
#include <functional>

#include "utils.hpp"
#include "value.hpp"

namespace dice 
{
    // type conversions
    class conversions
    {
    public:
        using value_type = std::unique_ptr<base_value>;
        using cost_type = std::size_t;

        // cost of an impossible conversion
        static const cost_type max_cost;

        /** Find conversion cost from type <from> to type <to>
         * @param id of the <from> type
         * @param id of the <to> type
         * @return cost of the conversion 
         *         0 => no conversion needed,
         *         max_cost => the conversion is not supported
         */
        cost_type cost(type_id from, type_id to) const;

        /** Convert value to type <to>
         * @param id of the <to> type
         * @param value to convert
         * @return converted value
         */ 
        value_type convert(type_id to, value_type&& value) const;
    };

    class conversion_visitor : public value_visitor
    {
    public:
        using value_type = std::unique_ptr<base_value>;

        conversion_visitor(type_id result_type) : result_type_(result_type) {}

        void visit(type_int* value) override 
        {
            if (result_type_ == type_id::floating_point)
                converted_value_ = make<type_double>(
                    static_cast<double>(value->data())
                );
            else if (result_type_ == type_id::random_variable)
            {
                converted_value_ = make<type_rand_var>(
                    constant_tag{}, value->data()
                );
            }
        }

        void visit(type_double*) override {}
        void visit(type_rand_var*) override {}

        value_type value() { return std::move(converted_value_); }
    private:
        value_type converted_value_;
        type_id result_type_;
    };

}