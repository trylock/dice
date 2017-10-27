#ifndef DICE_VALUE_HPP_
#define DICE_VALUE_HPP_

#include <memory>
#include <string>
#include <typeinfo>

#include "random_variable.hpp"
#include "random_variable_decomposition.hpp"

namespace dice 
{
    // Type of the type identifier of a value in a dice expression
    using type_id = std::string;

    // Parent of all value types that are used in a dice expressions
    class base_value 
    {
    public:
        virtual ~base_value() {}

        /** Get type of this value.
         * It has to be unique for each type.
         * It has to be the same for the same type 
         * (i.e. multiple values of the same type will return the same type id)
         */
        virtual type_id type() const = 0;

        /** Create a copy of this value.
         * @return copied value 
         */
        virtual std::unique_ptr<base_value> clone() const = 0;
    };

    // Value with data
    template<typename T>
    class typed_value : public base_value
    {
    public:
        using value_type = T;

        static type_id id() 
        {
            return typeid(typed_value<value_type>).name();
        }

        typed_value() {}
        explicit typed_value(const value_type& value) : value_(value) {}
        explicit typed_value(value_type&& value) : value_(std::move(value)) {}

        // get type id
        type_id type() const override { return id(); }

        // copy value
        std::unique_ptr<base_value> clone() const override
        {
            return std::make_unique<typed_value>(data());
        }

        // value getter setter
        value_type& data() { return value_; }
        const value_type& data() const { return value_; }
    private:
        value_type value_;
    };

    // used data types
    using type_int = typed_value<int>;
    using type_double = typed_value<double>;
    using type_rand_var = typed_value<
        random_variable_decomposition<int, double>>;

    template<typename T, typename... Value>
    std::unique_ptr<T> make(Value&&... data)
    {
        static_assert(std::is_base_of<base_value, T>::value, 
            "Dice value has to be derived from dice::base_value.");
        using value_type = typename T::value_type;
        return std::make_unique<T>(value_type{ std::forward<Value>(data)... });
    }
}

#endif // DICE_VALUE_HPP_