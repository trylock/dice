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
        using fn = function_traits;

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
        void add_function(const std::string& name, user_function&& func);

        /** Call a function with arguments given by argument iterators
         * @param iterator pointing to the first argument
         * @param iterator pointing to the last argument
         * @return computed value
         */
        fn::return_type call_var(
            const std::string& name, 
            fn::args_iterator first, 
            fn::args_iterator last);

        /** Call a function with the same arguments
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
            if (argc_ >= args_.size())
                throw compiler_error(
                    name + "(): exceeded maximal number of arguments");

            args_[argc_++] = std::forward<Arg>(first_arg);
            return call(name, std::forward<Args>(rest)...);
        }

        inline value_type call(const std::string& name)
        {
            auto result = call_var(
                name, 
                args_.begin(), 
                args_.begin() + argc_);
            argc_ = 0;
            return result;
        }

    private:
        // type conversions
        conversions conversions_;
        // available functions    
        std::unordered_map<std::string, std::vector<user_function>> functions_;
        // available variables
        std::unordered_map<std::string, value_type> variables_;

        // argument value storage
        std::vector<value_type> args_;
        std::size_t argc_ = 0;

    };
}

#endif // DICE_LIBRARY_HPP_