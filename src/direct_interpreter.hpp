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
    // compute decomposition of a random variable
    class decomposition_visitor : public value_visitor
    {
    public:
        inline void visit(type_int*) override {}
        inline void visit(type_double*) override {}
        inline void visit(type_rand_var* var) override 
        {
            if (!var->data().has_dependencies())
                var->data().compute_decomposition();
        }
    };

    // count random variables with dependencies
    class dependencies_visitor : public value_visitor
    {
    public:
        inline void visit(type_int*) override {}
        inline void visit(type_double*) override {}
        inline void visit(type_rand_var* var) override 
        {
            if (var->data().has_dependencies())
                ++counter_;
        }

        inline std::size_t count() const { return counter_; }
    private:
        std::size_t counter_ = 0;
    };

    /** Direct interpreter is a dice expressions interpreter that evaluates
     * expressions as soon as they are parsed. It does not use any itermediate
     * representation.
     */
    template<typename Environment>
    class direct_interpreter
    {
    public:
        using value_type = std::unique_ptr<base_value>;
        using value_list = std::vector<value_type>;

        explicit direct_interpreter(Environment* env) : env_(env) {}

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
            dependencies_visitor deps;
            lhs->accept(&deps);
            rhs->accept(&deps);
            if (deps.count() > 0)
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
            decomposition_visitor decomp;
            value->accept(&decomp);
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
                dependencies_visitor deps;
                for (auto&& arg : args)
                {
                    arg->accept(&deps);
                    if (deps.count() > 0)
                        break;
                }

                // if there is such a parameter, compute decomposition for
                // all random variables in the list
                if (deps.count() > 0)
                {
                    decomposition_visitor decomp;
                    for (auto&& arg : args)
                    {
                        arg->accept(&decomp);
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

            dependencies_visitor deps;
            lhs->accept(&deps);
            rhs->accept(&deps);
            if (deps.count() > 0)
            {
                decomposition_visitor decomp;
                lhs->accept(&decomp);
                rhs->accept(&decomp);
            }
        }
    };
}

#endif // DICE_INTERPRETER_HPP_