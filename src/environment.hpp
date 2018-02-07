#ifndef DICE_LIBRARY_HPP_
#define DICE_LIBRARY_HPP_

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <cassert>
#include <random>
#include <algorithm>

#include "safe.hpp"
#include "value.hpp"
#include "utils.hpp"
#include "functions.hpp"
#include "conversions.hpp"
#include "random_variable.hpp"

namespace dice
{
    /** @brief An error thrown when there is an error in computation during 
     * script evaluation.
     */
    class compiler_error : public std::logic_error
    {
    public:
        explicit compiler_error(const std::string& message) : 
            std::logic_error(message) {}
    };

    /** @brief Symbol table.
     *
     * This class manages variables and functions.
     * 
     * Example variable usage:
     * @code
     * environment e;
     * e.set_var("test_var", make<type_int>(4));
     * auto test_var = e.get_var("test_var");
     * @endcode
     * 
     * Example function call:
     * @code
     * environment e;
     * auto indicator_in_4_to_6 = e.call(
     *      "in", 
     *      make<type_rand_var>(...), 
     *      make<type_int>(4), 
     *      make<type_int>(6));
     * @endcode
     */
    class environment 
    {
    public:
        using value_type = std::unique_ptr<base_value>;
        using fn = execution_context;

        environment();

        /** @brief Set value of a variable.
         *
         * @param name of a variable
         * @param value of the variable
         */
        void set_var(const std::string& name, value_type value);

        /** @brief Get value of a variable.
         *
         * @param name of a variable
         * 
         * @return pointer to the value of this variable or 
         *         nullptr if it does not exist
         */
        base_value* get_var(const std::string& name);

        /** @brief Get value of a variable.
         *
         * @param name of a variable
         * 
         * @return constant pointer to the value of this variable or 
         *         nullptr if it does not exist
         */
        const base_value* get_var(const std::string& name) const;

        /** @brief Add a function to the environment.
         *
         * Added function will be available in dice expressions.
         * 
         * @param name of the function
         * @param function implementation and metadata
         */
        void add_function(const std::string& name, function_definition function);

        /** @brief Call a function with the same arguments.
         * 
         * @param name of the function
         * @param first_arg first argument
         * @param rest of the arguments passed to the function
         * 
         * @return computed value
         * 
         * @throws compiler_error when an error occurs during the function call
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

        /** @brief Call function without arguments.
         *
         * It will clear the args_ vector after the call.
         *
         * @attention arguments in the args_ vector are still passed to the call
         * 
         * @param name of the function
         * 
         * @return computed result
         */
        inline value_type call(const std::string& name)
        {
            try 
            {
                auto value = call_var(name, args_.begin(), args_.end());
                args_.clear();
                return value;
            }
            catch (...)
            {
                args_.clear();
                throw;
            }
        }

        /** Call a function with arguments in a list.
         * @param name of the function
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
         * @param name of the function
         * @param context of execution of this call
         * @return computed value
         */
        fn::return_type call_prepared(
            const std::string& name, 
            execution_context& context);
    };
}

#endif // DICE_LIBRARY_HPP_