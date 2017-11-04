#ifndef DICE_INTERPRETER_HPP_
#define DICE_INTERPRETER_HPP_

#include <array>
#include <memory>
#include <sstream>

#include "value.hpp"
#include "symbols.hpp"
#include "environment.hpp"

namespace dice 
{
    template<typename Environment>
    class interpreter
    {
    public:
        using value_type = std::unique_ptr<base_value>;
        using value_list = std::vector<value_type>;

        explicit interpreter(Environment* env) : env_(env) {}

        void enter_assign()
        {
            is_definition_ = true;
        }

        void leave_assign()
        {
            is_definition_ = false;
        }

        /** Create a new default value.
         * This is used when there is a parsing error.
         * @return default value
         */
        value_type make_default()
        {
            return make<type_int>(0);
        }

        /** Interpret number as an int or double.
         * @param string representation of the number
         * @return number's value
         */
        value_type number(const std::string& value)
        {    
            if (value.find('.') != std::string::npos)
            {
                return make<type_double>(std::atof(value.c_str()));
            }
            else 
            {
                return make<type_int>(std::atoi(value.c_str()));
            }
        }

        /** Interpret a variable name.
         * @param name of a variable
         * @return value of the variable on nullptr if it does not exist
         */
        value_type variable(const std::string& name)
        {
            auto value = env_->get_var(name);
            if (value == nullptr)
            {
                throw compiler_error("Unknown variable '" + name + "'");
            }

            return value->clone();
        }

        /** Add left hand side to the right hand side
         * @param left operand
         * @param right operand
         * @return sum
         */
        value_type add(value_type lhs, value_type rhs)
        {
            process_children(lhs.get(), rhs.get());
            return env_->call("+", std::move(lhs), std::move(rhs));
        }

        /** Subtract right hand side from the left hand side
         * @param left operand
         * @param right operand
         * @return difference
         */
        value_type sub(value_type lhs, value_type rhs)
        {
            process_children(lhs.get(), rhs.get());
            return env_->call("-", std::move(lhs), std::move(rhs));
        }

        /** Multiply left hand side with the right hand side
         * @param left operand
         * @param right operand
         * @return product
         */
        value_type mult(value_type lhs, value_type rhs)
        {
            process_children(lhs.get(), rhs.get());
            return env_->call("*", std::move(lhs), std::move(rhs));
        }

        /** Divide left hand side with the right hand side
         * @param left operand
         * @param right operand
         * @return division
         */
        value_type div(value_type lhs, value_type rhs)
        {
            process_children(lhs.get(), rhs.get());
            return env_->call("/", std::move(lhs), std::move(rhs));
        }

        /** Negate value.
         * @param original value
         * @return negation of the value
         */
        value_type unary_minus(value_type value)
        {
            return env_->call("unary-", std::move(value));
        }

        /** Compute a binary relational operator.
         * @param operator name
         * @param left operand
         * @param right operand
         * @return result
         */
        value_type rel_op(const std::string& type, value_type lhs, 
            value_type rhs)
        {
            process_children(lhs.get(), rhs.get());
            return env_->call(type, std::move(lhs), std::move(rhs));
        }

        /** Compute relational in operator.
         * @param value 
         * @param lower bound of the interval
         * @param upper bound of the interval
         * @return result of the in operator
         */
        value_type rel_in(value_type var, value_type lower_bound, 
            value_type upper_bound)
        {
            return env_->call("in", 
                std::move(var), 
                std::move(lower_bound), 
                std::move(upper_bound));
        }

        /** Compute a roll operator.
         * @param left operand
         * @param right operand
         * @return result of the operator
         */
        value_type roll(value_type lhs, value_type rhs)
        {
            // check that none of the operands depends on a random variable
            if (has_dependencies(lhs) || has_dependencies(rhs))
            {
                throw compiler_error(
                    "It is invalid to use names in dice roll operator");
            }
            return env_->call("__roll_op", std::move(lhs), std::move(rhs));
        }

        /** Assing value to variable with given name.
         * @param variable name
         * @param new value of the variable
         * @return nullptr
         */
        value_type assign(const std::string& name, value_type value)
        {
            compute_decomposition(value);
            env_->set_var(name, std::move(value));
            return nullptr;
        }

        /** Call a function with given arguments.
         * @param function name
         * @param argument list
         * @return result of the function call
         */
        value_type call(const std::string& name, value_list&& args)
        {
            if (is_definition_)
            {
                // check whether there is a parameter that depends on 
                // a random variable
                bool convert = false;
                for (auto&& arg : args)
                {
                    if (has_dependencies(arg))
                    {
                        convert = true;
                        break;
                    }
                }

                // if there is such a parameter, compute decomposition for
                // all random variables in the list
                if (convert)
                {
                    for (auto&& arg : args)
                    {
                        compute_decomposition(arg);
                    }
                }
            }

            return env_->call_var(name, std::move(args));
        }

    private:
        Environment* env_;
        bool is_definition_ = false;

        /** Process children of a binary node. 
         * @param left child
         * @param right child
         */
        void process_children(base_value* lhs, base_value* rhs)
        {
            // don't convert random variables if we're not in a name definition
            if (!is_definition_)
                return;

            auto var_a = dynamic_cast<type_rand_var*>(lhs);
            if (var_a == nullptr)
                return;
            auto var_b = dynamic_cast<type_rand_var*>(rhs);
            if (var_b == nullptr)
                return;

            if (!var_a->data().has_dependencies() && 
                !var_b->data().has_dependencies())
                return;
            
            if (!var_a->data().has_dependencies())
                var_a->data().compute_decomposition();
            else if (!var_b->data().has_dependencies())
                var_b->data().compute_decomposition();
        }

        /** Check whether a random variable has dependencies.
         * @param arbitrary value (doesn't have to be a random variable)
         * @return true iff the value is a random variable and 
         *         it has dependencies
         */
        template<typename ValuePtr>
        bool has_dependencies(ValuePtr&& value)
        {
            auto var = dynamic_cast<type_rand_var*>(&*value);
            return var != nullptr && var->data().has_dependencies();
        }

        /** Compute decomposition of a random variable.
         * Noop if the value is not a random variable.
         * @param arbitrary value (doesn't have to be a random variable)
         */
        template<typename ValuePtr>
        void compute_decomposition(ValuePtr&& value)
        {
            auto var = dynamic_cast<type_rand_var*>(&*value);
            if (var != nullptr && !var->data().has_dependencies())
            {
                var->data().compute_decomposition();
            }
        }
    };
}

#endif // DICE_INTERPRETER_HPP_