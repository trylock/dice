#ifndef DICE_LIBRARY_HPP_
#define DICE_LIBRARY_HPP_

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "utils.hpp"
#include "value.hpp"
#include "random_variable.hpp"

namespace dice
{
    class compiler_error : public std::runtime_error
    {
    public:
        explicit compiler_error(const std::string& message) : std::runtime_error(message) {}
    };

    // object representing a function callable from a dice expression
    class user_function
    {
    public:
        using arg_type = std::unique_ptr<base_value>;
        using return_type = std::unique_ptr<base_value>;
        using args_iterator = typename std::vector<arg_type>::iterator;
        using callable_type = std::function<return_type(args_iterator, args_iterator)>;

        user_function() {}
        user_function(callable_type callable) : callable_(callable) {}
        user_function(callable_type callable, std::vector<type_id>&& arg_types) : 
            callable_(callable), arg_types_(std::move(arg_types)) {}

        /** Call this user function with given arguments 
         * @param argument iterator pointing at the first argument
         * @param argument iterator pointing at the last argument
         * @return result of the call 
         */
        inline return_type operator()(args_iterator first, args_iterator last) const
        {
            return callable_(first, last);
        }

        inline const std::vector<type_id>& arg_types() const 
        {
            return arg_types_; 
        }
    private:
        // code of the function
        callable_type callable_;
        // argument types for type checking
        std::vector<type_id> arg_types_;
    };

    // helper checks that are used in user functions code
    class function_traits
    {
    public:
        static const std::size_t max_argc = 4;

        template<typename ExpectedType>
        static ExpectedType* arg(user_function::args_iterator it) 
        {
            auto result = dynamic_cast<ExpectedType*>(it->get());
            if (result == nullptr)
                throw compiler_error(
                    std::string("Invalid argument type. Expected ") + typeid(ExpectedType).name());
            return result;
        }

        inline static std::size_t argc(
            user_function::args_iterator first, 
            user_function::args_iterator last)
        {
            return std::distance(first, last);
        }
    };

    // type conversions
    class conversions
    {
    public:
        using value_ptr = std::unique_ptr<base_value>;
        using key_type = std::pair<type_id, type_id>;
        using cost_type = std::size_t;

        struct conversion
        {
            std::function<value_ptr(value_ptr)> func;
            cost_type cost = 0;
        };

        // cost of an impossible conversion
        static cost_type max_cost;

        template<typename Func>
        void add_conversion(type_id from, type_id to, Func&& func)
        {
            convert_.insert(std::make_pair(std::make_pair(from, to), std::forward<Func>(func)));
        }

        /** Find conversion cost from type <from> to type <to>
         * @param id of the <from> type
         * @param id of the <to> type
         * @return cost of the conversion 
         *         0 => no conversion needed,
         *         max_cost => the conversion is not supported
         */
        cost_type cost(type_id from, type_id to) const;

        /** Convert value from type <from> to type <to>
         * @param id of the <from> type
         * @param id of the <to> type
         * @param value to convert
         * @return converted value
         */ 
        value_ptr convert(type_id from, type_id to, value_ptr&& value) const;
    private:
        std::unordered_map<key_type, conversion, pair_hash<type_id, type_id>> convert_;
    };

    // simple symbol table 
    class environment 
    {
    public:
        using value_type = std::unique_ptr<base_value>;

        environment();

        /** Call a function with arguments given by argument iterators
         * @param iterator pointing to the first argument
         * @param iterator pointing to the last argument
         * @return computer value
         */
        user_function::return_type call_var(
            const std::string& name, 
            user_function::args_iterator first, 
            user_function::args_iterator last);

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
                    name + "(): maximal number of arguments exceeded");

            args_[argc_++] = std::forward<value_type>(first_arg);
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
        // available functions    
        std::unordered_map<std::string, user_function> functions_;

        // argument value storage
        std::vector<value_type> args_;
        std::size_t argc_;

    };
}

#endif // DICE_LIBRARY_HPP_