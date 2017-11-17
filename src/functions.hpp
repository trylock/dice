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
        value_type& raw_arg(std::size_t i)
        {
            assert(i < argc());
            return *(first_ + i);
        }
        
        /** Get a raw value of ith argument of current function.
         * @param index of an argument
         * @return value of the argument
         */
        const value_type& raw_arg(std::size_t i) const
        {
            assert(i < argc());
            return *(first_ + i);
        }

        /** Convert ith argument to given type and return the pointer.
         * @param index of an argument
         * @return converted ith argument
         */
        template<typename ExpectedType>
        ExpectedType* arg(std::size_t i)
        {
            auto result = dynamic_cast<ExpectedType*>(raw_arg(i).get());
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
        type_id arg_type(std::size_t i) const
        {
            return raw_arg(i)->type();
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
            callable_(callable), args_(std::move(arg_types)) {}

        /** Call this user function with given arguments 
         * @param argument iterator pointing at the first argument
         * @param argument iterator pointing at the last argument
         * @return result of the call 
         */
        inline fn::return_type operator()(fn::context_type& context) const
        {
            return callable_(context);
        }

        inline const std::vector<type_id>& args() const 
        {
            return args_; 
        }

        inline std::size_t argc() const { return args_.size(); }
        
    private:
        // code of the function
        fn::callable_type callable_;
        // argument types for type checking
        std::vector<type_id> args_;
    };
}