#ifndef DICE_INTERPRETER_HPP_
#define DICE_INTERPRETER_HPP_

#include <memory>

#include "value.hpp"
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
        value_type add(value_type&& lhs, value_type&& rhs)
        {
            return env_->call("+", std::move(lhs), std::move(rhs));
        }

        /** Subtract right hand side from the left hand side
         * @param left operand
         * @param right operand
         * @return difference
         */
        value_type sub(value_type&& lhs, value_type&& rhs)
        {
            return env_->call("-", std::move(lhs), std::move(rhs));
        }

        /** Multiply left hand side with the right hand side
         * @param left operand
         * @param right operand
         * @return product
         */
        value_type mult(value_type&& lhs, value_type&& rhs)
        {
            return env_->call("*", std::move(lhs), std::move(rhs));
        }

        /** Divide left hand side with the right hand side
         * @param left operand
         * @param right operand
         * @return division
         */
        value_type div(value_type&& lhs, value_type&& rhs)
        {
            return env_->call("/", std::move(lhs), std::move(rhs));
        }

        /** Negate value.
         * @param original value
         * @return negation of the value
         */
        value_type unary_minus(value_type&& value)
        {
            return env_->call("unary-", std::move(value));
        }

        /** Compute a binary relational operator.
         * @param operator name
         * @param left operand
         * @param right operand
         * @return result
         */
        value_type rel_op(const std::string& type, value_type&& lhs, 
            value_type&& rhs)
        {
            return env_->call(type, std::move(lhs), std::move(rhs));
        }

        /** Compute relational in operator.
         * @param value 
         * @param lower bound of the interval
         * @param upper bound of the interval
         * @return result of the in operator
         */
        value_type rel_in(value_type&& var, value_type&& lower_bound, 
            value_type&& upper_bound)
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
        value_type roll(value_type&& lhs, value_type&& rhs)
        {
            return env_->call("__roll_op", std::move(lhs), std::move(rhs));
        }

        /** Assing value to variable with given name.
         * @param variable name
         * @param new value of the variable
         * @return nullptr
         */
        value_type assign(const std::string& name, value_type&& value)
        {
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
            return env_->call_var(name, args.begin(), args.end());
        }

        // Process result(s)) of a whole production

        /** Process the whole result.
         * @param list of computed values
         * @return list of results
         */
        value_list process_stmts(value_list&& values)
        {
            return std::move(values);
        }

        /** Process the result of a statement production.
         * @param computed value
         * @return pass the value unchanged
         */
        value_type process_stmt(value_type&& value)
        {
            return std::move(value);
        }
        
        /** Process the result of an expression production.
         * @param computed value
         * @return pass the value unchanged
         */
        value_type process_expr(value_type&& value)
        {
            return std::move(value);
        }
        
        /** Process the result of an addition production.
         * @param computed value
         * @return pass the value unchanged
         */
        value_type process_add(value_type&& value)
        {
            return std::move(value);
        }
        
        /** Process the result of a multiplication production.
         * @param computed value
         * @return pass the value unchanged
         */
        value_type process_mult(value_type&& value)
        {
            return std::move(value);
        }

        /** Process the result of the dice_roll production.
         * @param computed value
         * @return postprocessed value
         */
        value_type process_roll(value_type&& value)
        {
            if (is_definition_)
                return conv_var(std::move(value));
            return std::move(value);
        }
        
        /** Process the result of a factor production.
         * @param computed value
         * @return pass the value unchanged
         */
        value_type process_factor(value_type&& value)
        {
            return std::move(value);
        }
        
        /** Process the result of a param list production.
         * @param computed value
         * @return pass the value unchanged
         */
        value_list process_param_list(value_list&& value)
        {
            return std::move(value);
        }

    private:
        Environment* env_;
        bool is_definition_ = false;

        value_type conv_var(value_type&& value)
        {
            auto var = dynamic_cast<type_rand_var*>(value.get());
            if (var != nullptr && !var->data().has_dependencies())
            {
                var->data().compute_decomposition();
            }
            return std::move(value);
        }
    };
}

#endif // DICE_INTERPRETER_HPP_