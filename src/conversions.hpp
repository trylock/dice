#ifndef DICE_CONVERSIONS_HPP_
#define DICE_CONVERSIONS_HPP_

#include <memory>
#include <unordered_map>
#include <functional>

#include "utils.hpp"
#include "value.hpp"

namespace dice 
{
    /** @brief This class manages conversions of values in dice expression.
     *
     * Example usage:
     * @code
     * conversions c;
     * conversions::cost_type cost_int_to_real = c.cost(type_id::integer, type_id::real);
     * conversions::value_type real_value = c.convert(type_id::real, make<type_int>(14));
     * @endcode
     */
    class conversions
    {
    public:
        using value_type = std::unique_ptr<base_value>;
        using cost_type = std::size_t;

        // cost of an impossible conversion
        static const cost_type max_cost;

        /** @brief Find conversion cost from type \p from to type \p to
         *
         * @param from type id
         * @param to type id
         * 
         * @return cost of the conversion.
         *         0 => no conversion needed,
         *         max_cost => the conversion is not supported
         */
        cost_type cost(type_id from, type_id to) const;

        /** @brief Convert value to type \p to
         *
         * @param to type id
         * @param value to convert
         * 
         * @return converted value
         */ 
        value_type convert(type_id to, value_type value) const;
    };

    /** @internal */
    class conversion_visitor : public value_visitor
    {
    public:
        using value_type = std::unique_ptr<base_value>;

        explicit conversion_visitor(type_id result_type) : result_type_(result_type) {}

        // convert int to the result type
        void visit(type_int* value) override;

        // there is no implicit conversion from a real
        void visit(type_real*) override {}

        // there is no implicit conversion from a random variable
        void visit(type_rand_var*) override {}

        // get converted value
        value_type value() { return std::move(converted_value_); }
    private:
        value_type converted_value_;
        type_id result_type_;
    };
}

#endif // DICE_CONVERSIONS_HPP_