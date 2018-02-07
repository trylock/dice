#ifndef DICE_FUNCTIONS_HPP_
#define DICE_FUNCTIONS_HPP_

#include <memory>
#include <vector>
#include <functional>
#include <cassert>

#include "value.hpp"

namespace dice 
{
    /** @brief Context of dice user function execution.
     * 
     * This is the only argument passed to every user function call.
     * 
     * Example usage:
     * @code
     * execution_context context{ ... };
     * auto first_arg = context.arg<type_int>(0);
     * auto second_arg = context.arg<type_rand_var>(1);
     * @endcode 
     */
    class execution_context
    {
    public:
        using value_type = std::unique_ptr<base_value>;
        using return_type = std::unique_ptr<base_value>;
        using context_type = execution_context;
        using callable_type = std::function<return_type(context_type&)>;
        using arg_iterator = typename std::vector<value_type>::iterator;

        execution_context() = default;

        execution_context(arg_iterator first, arg_iterator last) : 
            first_(first), last_(last) {}

        /** @brief Get a raw value of ith argument of current function.
         *
         * @param index of an argument
         * 
         * @return value of the argument
         */
        value_type& raw_arg(std::size_t index)
        {
            assert(index < argc());
            return *(first_ + index);
        }
        
        /** @brief Get a raw value of ith argument of current function.
         *
         * @param index of an argument
         * 
         * @return value of the argument
         */
        const value_type& raw_arg(std::size_t index) const
        {
            assert(index < argc());
            return *(first_ + index);
        }

        /** @brief Convert ith argument to given type and return the pointer.
         *
         * @tparam ExpectedType type to which we should convert the ith argumnet.
         *                      It has to be derived from the base_value type.
         *
         * @param index of an argument
         * 
         * @return pointer to converted ith argument
         * 
         * @throws std::invalid_argument if the argument is not convertible to 
         *                               ExpectedType.
         */
        template<typename ExpectedType>
        ExpectedType* arg(std::size_t index)
        {
            static_assert(
                std::is_base_of<base_value, ExpectedType>::value, 
                "ExpectedType has to be derived from dice::base_value");

            auto result = dynamic_cast<ExpectedType*>(raw_arg(index).get());
            if (result == nullptr)
                throw std::invalid_argument(
                    std::string("Invalid argument type. Expected ") + 
                    to_string(ExpectedType::id()));
            return result;
        }
        
        /** @brief Get type of the ith argument.
         *
         * @param index of an argument
         * 
         * @return argument type
         */
        type_id arg_type(std::size_t index) const
        {
            return raw_arg(index)->type();
        }

        /** @brief Number of arguments of this function.
         *
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

    /** @brief Object representing a function callable from a dice expression
     *
     * Example - function metadata:
     * @code 
     * function_definition func{ ..., { type_id::integer, type_id::real } };
     * assert(func.argc() == 2);
     * assert(func.arg_type(0) == type_id::integer);
     * assert(func.arg_type(1) == type_id::real);
     * @endcode
     * 
     * Example - call function:
     * @code
     * std::vector<execution_context::value_type> args{ 
     *      make<type_int>(1), 
     *      make<type_real>(3.1415) 
     * };
     * execution_context context{ args.begin(), args.end() };
     * func(context);
     * @endcode
     */
    class function_definition
    {
    public:
        using fn = execution_context;

        function_definition() = default;
        ~function_definition() = default;

        /** @brief Create function without arguments.
         *
         * @param callable function implementation.
         *        It takes execution_context::value_type type as an argument.
         *        it returns execution_context::return_type type.
         */
        explicit function_definition(fn::callable_type callable) : 
            callable_(std::move(callable)) {}

        /** @brief Create function with arguments
         *
         * @param callable function implementation.
         *        It takes execution_context::value_type type as an argument.
         *        It returns execution_context::return_type type.
         * @param arg_types vector of argument types of this function.
         */
        function_definition(
            fn::callable_type callable, 
            std::vector<type_id>&& arg_types) : 
            callable_(std::move(callable)), arg_types_(std::move(arg_types)) {}

        // disallow copy
        function_definition(const function_definition&) = delete;
        void operator=(const function_definition&) = delete;

        // allow move
        function_definition(function_definition&&) = default;
        function_definition& operator=(function_definition&&) = default;

        /** @brief Call this user function with given arguments 
         *
         * @param context of execution of this call
         * 
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