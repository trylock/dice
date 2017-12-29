#ifndef DICE_FUNCTIONS_HPP_
#define DICE_FUNCTIONS_HPP_

#include <memory>
#include <vector>
#include <functional>
#include <cassert>

#include "value.hpp"

namespace dice 
{
    // Context of a function execution
    class execution_context
    {
    public:
        using value_type = std::unique_ptr<base_value>;
        using return_type = std::unique_ptr<base_value>;
        using context_type = execution_context;
        using callable_type = std::function<return_type(context_type&)>;
        using arg_iterator = typename std::vector<value_type>::iterator;

        execution_context() {}

        execution_context(arg_iterator first, arg_iterator last) : 
            first_(first), last_(last) {}

        /** Get a raw value of ith argument of current function.
         * @param index of an argument
         * @return value of the argument
         */
        value_type& raw_arg(std::size_t index)
        {
            assert(index < argc());
            return *(first_ + index);
        }
        
        /** Get a raw value of ith argument of current function.
         * @param index of an argument
         * @return value of the argument
         */
        const value_type& raw_arg(std::size_t index) const
        {
            assert(index < argc());
            return *(first_ + index);
        }

        /** Convert ith argument to given type and return the pointer.
         * @param index of an argument
         * @return converted ith argument
         */
        template<typename ExpectedType>
        ExpectedType* arg(std::size_t index)
        {
            auto result = dynamic_cast<ExpectedType*>(raw_arg(index).get());
            if (result == nullptr)
                throw std::runtime_error(
                    std::string("Invalid argument type. Expected ") + 
                    to_string(ExpectedType::id()));
            return result;
        }
        
        /** Get type of the ith argument.
         * @param index of an argument
         * @return argument type
         */
        type_id arg_type(std::size_t index) const
        {
            return raw_arg(index)->type();
        }

        /** Number of arguments of this function.
         * @return number of arguments of this function call 
         */
        std::size_t argc() const
        {
            return std::distance(first_, last_);
        }

    private:
        arg_iterator first_;
        arg_iterator last_;
    };

    // object representing a function callable from a dice expression
    class function_definition
    {
    public:
        using fn = execution_context;

        function_definition() {}
        function_definition(fn::callable_type callable) : callable_(callable) {}
        function_definition(
            fn::callable_type callable, 
            std::vector<type_id>&& arg_types) : 
            callable_(callable), arg_types_(std::move(arg_types)) {}

        // disallow copy
        function_definition(const function_definition&) = delete;
        void operator=(const function_definition&) = delete;

        // allow move
        function_definition(function_definition&&) = default;
        function_definition& operator=(function_definition&&) = default;

        /** Call this user function with given arguments 
         * @param context of execution of this call
         * @return result of the call 
         */
        inline fn::return_type operator()(fn::context_type& context) const
        {
            return callable_(context);
        }

        /** Get type of the ith argument.
         * @param index of an argument
         * @return type of the argument
         */
        inline type_id arg_type(std::size_t index) const 
        {
            assert(index < argc());
            return arg_types_[index]; 
        }

        /** Get number of function arguments.
         * @return number of arguemtns of this function
         */
        inline std::size_t argc() const { return arg_types_.size(); }

    private:
        // code of the function
        fn::callable_type callable_;
        // argument types for type checking
        std::vector<type_id> arg_types_;
    };
}

#endif // DICE_FUNCTIONS_HPP_