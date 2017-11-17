#ifndef DICE_LIBRARY_HPP_
#define DICE_LIBRARY_HPP_

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <cassert>
#include <random>
#include <algorithm>

#include "value.hpp"
#include "utils.hpp"
#include "functions.hpp"
#include "conversions.hpp"
#include "random_variable.hpp"

namespace dice
{
    class compiler_error : public std::runtime_error
    {
    public:
        explicit compiler_error(const std::string& message) : 
            std::runtime_error(message) {}
    };

    // simple symbol table 
    class environment 
    {
    public:
        using value_type = std::unique_ptr<base_value>;
        using fn = execution_context;

        environment();

        /** Set value of a variable.
         * @param name of a variable
         * @param value of the variable
         */
        void set_var(const std::string& name, value_type&& value);

        /** Get value of a variable.
         * @param name of a variable
         * @return pointer to the value of this variable or 
         *         nullptr if it does not exist
         */
        base_value* get_var(const std::string& name);

        /** Get value of a variable.
         * @param name of a variable
         * @return constant pointer to the value of this variable or 
         *         nullptr if it does not exist
         */
        const base_value* get_var(const std::string& name) const;

        /** Add a function to the environment.
         * Added function will be available in dice expressions.
         * @param function name
         * @param function implementation and metadata
         */
        void add_function(const std::string& name, function_definition&& func);

        /** Call a function with the same arguments.
         * All functions can throw a compiler_error indicating a critical error
         * that the caller has to handle.
         * @param function name
         * @param arguments passed to the function
         * @return computed value
         */
        template<typename Arg, typename ...Args>
        value_type call(
            const std::string& name, 
            Arg&& first_arg, 
            Args&&... rest)
        {
            args_.push_back(std::forward<Arg>(first_arg));
            return call(name, std::forward<Args>(rest)...);
        }

        inline value_type call(const std::string& name)
        {
            auto value = call_var(name, args_.begin(), args_.end());
            args_.clear();
            return value;
        }

        /** Call a function with arguments in a list.
         * @param function name
         * @param first argument iterator
         * @param last argument iterator
         * @return computed value
         */
        inline value_type call_var(
            const std::string& name, 
            fn::arg_iterator first,
            fn::arg_iterator last)
        {
            execution_context context{ first, last };
            auto value = call_prepared(name, context);
            return value;
        }

    private:
        // type conversions
        conversions conversions_;
        // available functions    
        std::unordered_map<
            std::string, 
            std::vector<function_definition>> functions_;
        // available variables
        std::unordered_map<std::string, value_type> variables_;
        // auxiliary vector of function arguments
        std::vector<fn::value_type> args_;

        /** Call a function with prepared context.
         * @param function name
         * @param execution context of this call
         * @return computed value
         */
        fn::return_type call_prepared(
            const std::string& name, 
            execution_context& context);
    };
}

#endif // DICE_LIBRARY_HPP_