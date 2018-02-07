#ifndef DICE_INTERPRETER_HPP_
#define DICE_INTERPRETER_HPP_

#include <array>
#include <memory>
#include <sstream>
#include <cassert>

#include "value.hpp"
#include "symbols.hpp"
#include "environment.hpp"

namespace dice 
{
    /** @brief This interpreter evaluates expressions as soon as they are 
     *         parsed.
     * 
     * It does not use any itermediate representation.
     * 
     * @tparam Environment environment type
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

        /** @brief Create a new default value.
         *
         * This is used when there is a parsing error.
         * This will create a new value every time it is called.
         * 
         * @return default value
         */
        value_type make_default() const
        {
            return make<type_int>(0);
        }

        /** @brief Interpret number as an int or real.
         *
         * @param token of the number
         * 
         * @return number's value
         */
        value_type number(symbol& token) const
        {
            assert(token.type == symbol_type::number);
            return std::move(token.value);
        }

        /** @brief Interpret a variable name.
         *
         * It will find the variable in environment.
         *
         * @param name of a variable
         * 
         * @return value of the variable or nullptr if it does not exist
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

        /** @brief Add left hand side to the right hand side
         *
         * It will call the +(left, right) function in environment.
         *
         * @param left operand
         * @param right operand
         * 
         * @return sum
         * 
         * @throws compiler_error if argument types are invalid or an error 
         *                        occured during the computation.
         */
        value_type add(value_type left, value_type right)
        {
            prepare_operands(left.get(), right.get());
            return env_->call("+", std::move(left), std::move(right));
        }

        /** @brief Subtract right hand side from the left hand side.
         *
         * It will call the -(left, right) function in environment.
         *
         * @param left operand
         * @param right operand
         * 
         * @return difference
         * 
         * @throws compiler_error if argument types are invalid or an error 
         *                        occured during the computation.
         */
        value_type sub(value_type left, value_type right)
        {
            prepare_operands(left.get(), right.get());
            return env_->call("-", std::move(left), std::move(right));
        }

        /** @brief Multiply left hand side with the right hand side
         *
         * It will call the *(left, right) function in environment.
         *
         * @param left operand
         * @param right operand
         * 
         * @return product
         * 
         * @throws compiler_error if argument types are invalid or an error 
         *                        occured during the computation.
         */
        value_type mult(value_type left, value_type right)
        {
            prepare_operands(left.get(), right.get());
            return env_->call("*", std::move(left), std::move(right));
        }

        /** @brief Divide left hand side with the right hand side
         *
         * It will call the /(left, right) function in environment.
         *
         * @param left operand
         * @param right operand
         * 
         * @return division
         * 
         * @throws compiler_error if argument types are invalid or an error 
         *                        occured during the computation.
         */
        value_type div(value_type left, value_type right)
        {
            prepare_operands(left.get(), right.get());
            return env_->call("/", std::move(left), std::move(right));
        }

        /** @brief Negate value.
         *
         * It will call the unary-(value) function in environment.
         *
         * @param value
         * 
         * @return negation of the value
         * 
         * @throws compiler_error if argument types are invalid or an error 
         *                        occured during the computation.
         */
        value_type unary_minus(value_type value)
        {
            return env_->call("unary-", std::move(value));
        }

        /** @brief Compute a binary relational operator.
         *
         * It will call the <(left, right), <=(left, right), etc. function in 
         * environment.
         *
         * @param type of the operator (<, <=, ==, !=, >=, >)
         * @param left operand
         * @param right operand
         * 
         * @return result
         * 
         * @throws compiler_error if argument types are invalid or an error 
         *                        occured during the computation.
         */
        value_type rel_op(
            const std::string& type, 
            value_type left, 
            value_type right)
        {
            prepare_operands(left.get(), right.get());
            return env_->call(type, std::move(left), std::move(right));
        }

        /** @brief Compute relational in operator.
         *
         * It will call the in(value, lower_bound, upper_bound) function in 
         * environment.
         *
         * @param value to test
         * @param lower_bound of the interval
         * @param upper_bound of the interval
         * 
         * @return result of the in operator
         * 
         * @throws compiler_error if argument types are invalid or an error 
         *                        occured during the computation.
         */
        value_type rel_in(
            value_type value, 
            value_type lower_bound, 
            value_type upper_bound)
        {
            return env_->call("in", 
                std::move(value), 
                std::move(lower_bound), 
                std::move(upper_bound));
        }

        /** @brief Compute a roll operator.
         *
         * It will call the roll_op(left, right) function in environment.
         *
         * @param left operand
         * @param right operand
         * 
         * @return result of the operator
         * 
         * @throws compiler_error if argument types are invalid or an error 
         *                        occured during the computation.
         */
        value_type roll(value_type left, value_type right)
        {
            prepare_operands(left.get(), right.get());
            return env_->call("roll_op", std::move(left), std::move(right));
        }

        /** @brief Assing value to variable with given name.
         *
         * If a variable with given name already exists, a compiler_error 
         * exception will be thrown.
         * 
         * @param name of a new variable
         * @param value of the variable
         * 
         * @return nullptr
         * 
         * @throws compiler_error if the variable already exists and variable 
         *                        redefinition is disabled (see the 
         *                        set_variable_redefinition function)
         */
        value_type assign(const std::string& name, value_type value)
        {
            if (!variable_redefinition_ && env_->get_var(name) != nullptr)
            {
                throw compiler_error("Variable '" + name + "' redefinition.");
            }

            decomposition_visitor decomp;
            value->accept(&decomp);
            env_->set_var(name, std::move(value));
            is_definition_ = false;
            return nullptr;
        }

        /** @brief Call a function with given arguments.
         *
         * It will invoke function with the same name and arguments in 
         * environment.
         *
         * @param name of a function
         * @param arguments
         * 
         * @return result of the function call
         * 
         * @throws compiler_error if there is no such function or an error 
         *                        occured during the call.
         */
        value_type call(const std::string& name, value_list&& arguments)
        {
            if (is_definition_)
            {
                // check whether there is a parameter that depends on 
                // a random variable
                dependencies_visitor deps;
                for (auto&& arg : arguments)
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
                    for (auto&& arg : arguments)
                    {
                        arg->accept(&decomp);
                    }
                }
            }

            return env_->call_var(name, arguments.begin(), arguments.end());
        }

        /** @brief Enable/disable variable redefinition.
         *
         * @param value true iff the redefinition is allowed
         */
        void set_variable_redefinition(bool value)
        {
            variable_redefinition_ = value;
        }

        /** @brief Check whether the variable redefinition is allowed.
         *
         * @return true iff the redefinition is allowed
         */
        bool get_variable_redefinition() const
        {
            return variable_redefinition_;
        }
    private:
        Environment* env_;
        bool is_definition_ = false;
        bool variable_redefinition_ = false;

        // compute decomposition of a random variable
        class decomposition_visitor : public value_visitor
        {
        public:
            inline void visit(type_int*) override {}
            inline void visit(type_real*) override {}
            inline void visit(type_rand_var* var) override
            {
                var->data() = var->data().compute_decomposition();
            }
        };

        // count random variables with dependencies
        class dependencies_visitor : public value_visitor
        {
        public:
            inline void visit(type_int*) override {}
            inline void visit(type_real*) override {}
            inline void visit(type_rand_var* var) override
            {
                if (var->data().has_dependencies())
                    ++counter_;
            }

            inline std::size_t count() const { return counter_; }
        private:
            std::size_t counter_ = 0;
        };

        /** @brief Prepare operands of a binary operator.
         *
         * It computes deomposition for random variables.
         * It is doesn't do anything if we are not in variable definition or 
         * given values are not random variables.
         * 
         * @param left operand
         * @param right operand
         */
        void prepare_operands(base_value* left, base_value* right) const
        {
            // don't convert random variables if we're not in a name definition
            if (!is_definition_)
                return;

            dependencies_visitor deps;
            left->accept(&deps);
            right->accept(&deps);
            if (deps.count() > 0)
            {
                decomposition_visitor decomp;
                left->accept(&decomp);
                right->accept(&decomp);
            }
        }
    };
}

#endif // DICE_INTERPRETER_HPP_