#ifndef DICE_VALUE_HPP_
#define DICE_VALUE_HPP_

#include <memory>
#include <string>
#include <typeinfo>

#include "random_variable.hpp"
#include "random_variable_decomposition.hpp"

namespace dice 
{
    // Type identifier of a value in a dice expression
    enum class type_id
    {
        integer,
        floating_point,
        random_variable
    };

    // Translate C++ types to type_id
    template<typename ValueType>
    type_id get_type_id();

    template<>
    inline type_id get_type_id<int>() { return type_id::integer; }
    
    template<>
    inline type_id get_type_id<double>() { return type_id::floating_point; }

    template<>
    inline type_id get_type_id<random_variable_decomposition<int, double>>()
    {
        return type_id::random_variable;
    } 
    
    /** Convert type id to human readable string.
     * @param type id
     * @return name of the type
     */
    inline std::string to_string(type_id tid)
    {
        switch (tid)
        {
        case type_id::integer:
            return "int";
        case type_id::floating_point:
            return "double";
        case type_id::random_variable:
            return "random_variable";
        default:
            throw std::runtime_error(
                "Unknown type id: " + static_cast<int>(tid));
        }
    }

    template<typename T>
    class typed_value;

    // Visitor of a dice value
    class value_visitor
    {
    public:
        virtual ~value_visitor() {}

        virtual void visit(typed_value<int>* value) = 0;
        virtual void visit(typed_value<double>* value) = 0;
        virtual void visit(
            typed_value<random_variable_decomposition<int, double>>* value) = 0;
    };

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

        /** Visit this value using given visitor.
         * @param visitor
         */
        virtual void accept(value_visitor* visitor) = 0;
    };

    // Value with data
    template<typename T>
    class typed_value : public base_value
    {
    public:
        using value_type = T;

        static type_id id() 
        {
            return get_type_id<T>();
        }

        typed_value() {}
        explicit typed_value(const value_type& value) : value_(value) {}
        explicit typed_value(value_type&& value) : value_(std::move(value)) {}

        // get type id
        type_id type() const override { return id(); }

        // visit this value
        void accept(value_visitor* visitor) override
        {
            visitor->visit(this);
        }

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