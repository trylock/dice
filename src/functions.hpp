#include <memory>
#include <vector>
#include <functional>

#include "value.hpp"

namespace dice 
{
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
            callable_(callable), args_(std::move(arg_types)) {}

        /** Call this user function with given arguments 
         * @param argument iterator pointing at the first argument
         * @param argument iterator pointing at the last argument
         * @return result of the call 
         */
        inline return_type operator()(args_iterator first, args_iterator last) const
        {
            return callable_(first, last);
        }

        inline const std::vector<type_id>& args() const 
        {
            return args_; 
        }

        inline std::size_t argc() const { return args_.size(); }
        
    private:
        // code of the function
        callable_type callable_;
        // argument types for type checking
        std::vector<type_id> args_;
    };

    // helper checks that are used in user function's code
    class function_traits
    {
    public:
        static const std::size_t max_argc = 4;

        /** Check that an argument is of given type and convert it.
         * @param argument iterator
         * @return converted type
         */
        template<typename ExpectedType>
        static ExpectedType* arg(user_function::args_iterator it) 
        {
            auto result = dynamic_cast<ExpectedType*>(it->get());
            if (result == nullptr)
                throw std::runtime_error(
                    std::string("Invalid argument type. Expected ") + typeid(ExpectedType).name());
            return result;
        }

        /** Count number of arguments from arguments iterators
         * @param first argument iterator
         * @param last argument iterator
         * @return number of function arguments
         */
        inline static std::size_t argc(
            user_function::args_iterator first, 
            user_function::args_iterator last)
        {
            return std::distance(first, last);
        }
    };
}