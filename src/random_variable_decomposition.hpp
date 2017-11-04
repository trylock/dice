#ifndef DICE_RANDOM_VARIABLE_DECOMPOSITION_HPP_
#define DICE_RANDOM_VARIABLE_DECOMPOSITION_HPP_

#include <memory>
#include <vector>
#include <cassert>
#include <algorithm>

#include "utils.hpp"
#include "random_variable.hpp"

namespace dice
{
    /** Object representing a decomposition of a function of random variables.
     * 
     * Denote A a random variable. Let us assume that A depends on random 
     * variables X1,..., Xn. Then, from Law of total probability: 
     * P(A = k) = sum of 
     *      P(A = k | X1 = x1, ..., Xn = xn) * P(X1 = x1, ..., Xn = xn) 
     * for all x1, ..., xn
     * Because events `X1 = x1, ..., Xn = xn` are disjoint and they form 
     * the whole sample space (we are considering all such xi's)
     * 
     * Purpose of this is that variables `A | X1 = x1, ..., Xn = xn` might 
     * be independent.
     */
    template<typename ValueType, typename ProbabilityType>
    class random_variable_decomposition
    {
    public:
        using var_type = random_variable<ValueType, ProbabilityType>;
        using value_type = ValueType;

        random_variable_decomposition() {}

        explicit random_variable_decomposition(const var_type& variable)
        {
            vars_.push_back(variable);
        }

        explicit random_variable_decomposition(var_type&& variable)
        {
            vars_.push_back(std::move(variable));
        }
        
        // Random variable constructors

        random_variable_decomposition(
            std::vector<std::pair<value_type, std::size_t>>&& list)
            : random_variable_decomposition(var_type(std::move(list)))
        {
        }

        random_variable_decomposition(bernoulli_tag, ProbabilityType success)
            : random_variable_decomposition(var_type(bernoulli_tag{}, success))
        {
        }
        
        random_variable_decomposition(constant_tag, value_type value)
            : random_variable_decomposition(var_type(constant_tag{}, value))
        {
        }
        
        // allow copy
        random_variable_decomposition(
            const random_variable_decomposition&) = default;
        random_variable_decomposition& operator=(
            const random_variable_decomposition&) = default;

        // allow move
        random_variable_decomposition(
            random_variable_decomposition&&) = default;
        random_variable_decomposition& operator=(
            random_variable_decomposition&&) = default;

        /** Compute sum of 2 random variables.
         * Random variables don't need to be indendent.
         * @param right hand side of the operator
         * @return distribution of the sum
         */
        auto operator+(const random_variable_decomposition& other) const
        {
            return combine(other, [](auto&& var_a, auto&& var_b) 
            {
                return var_a + var_b;
            });
        }
        
        /** Subtract other random variable from this.
         * Random variables don't need to be indendent.
         * @param right hand side of the operator
         * @return result of the subtraction
         */
        auto operator-(const random_variable_decomposition& other) const
        {
            return combine(other, [](auto&& var_a, auto&& var_b) 
            {
                return var_a - var_b;
            });
        }
        
        /** Compute product of 2 random variables.
         * Random variables don't need to be indendent.
         * @param right hand side of the operator
         * @return result of the product
         */
        auto operator*(const random_variable_decomposition& other) const
        {
            return combine(other, [](auto&& var_a, auto&& var_b) 
            {
                return var_a * var_b;
            });
        }
        
        /** Divide this variable by other variable.
         * Random variables don't need to be indendent.
         * @param right hand side of the operator
         * @return result of the division
         */
        auto operator/(const random_variable_decomposition& other) const
        {
            return combine(other, [](auto&& var_a, auto&& var_b) 
            {
                return var_a / var_b;
            });
        }

        /** Multiply this random variable with -1.
         * @return negated random variable
         */
        auto operator-() const 
        {
            random_variable_decomposition result;
            result.deps_ = deps_;
            for (auto&& var : vars_)
            {
                result.vars_.push_back(-var);
            }
            return result;
        }

        /** Compute indicator: A < B (where A is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable B
         * @return indicator of A < B 
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto less_than(const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& var_a, auto&& var_b) 
            {
                return var_a.less_than(var_b);
            });
        }

        /** Compute indicator: A <= B (where A is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable B
         * @return indicator of A <= B 
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto less_than_or_equal(
            const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& var_a, auto&& var_b) 
            {
                return var_a.less_than_or_equal(var_b);
            });
        }

        /** Compute indicator: A = B (where A is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable B
         * @return indicator of A = B
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto equal(const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& var_a, auto&& var_b) 
            {
                return var_a.equal(var_b);
            });
        }

        /** Compute indicator: A != B (where A is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable Y
         * @return indicator of A != B 
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto not_equal(const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& var_a, auto&& var_b) 
            {
                return var_a.not_equal(var_b);
            });
        }

        /** Compute indicator: A > B (where A is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable B
         * @return indicator of A > B 
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto greater_than(const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& var_a, auto&& var_b) 
            {
                return var_a.greater_than(var_b);
            });
        }
        
        /** Compute indicator: A >= B (where A is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable Y
         * @return indicator of A >= YB
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto greater_than_or_equal(
            const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& var_a, auto&& var_b) 
            {
                return var_a.greater_than_or_equal(var_b);
            });
        }

        /** Compute indicator: A in [a, b] (where A is this random variable).
         * The interval is closed.
         * @param lower bound of the interval (a)
         * @param upper bound of the interval (b)
         * @return indicator of A in [a, b]
         * (i.e. a random variable with a bernoulli distribution)
         */
        template<typename T>
        auto in(T&& lower_bound, T&& upper_bound) const 
        {
            random_variable_decomposition result;
            result.deps_ = deps_;
            for (auto&& var : vars_)
            {
                result.vars_.push_back(var.in(lower_bound, upper_bound));
            }
            return result;
        }
        
        /** Roll num_rolls times with a dice of num_sides.
         * Random variables have to be independent.
         * @param number of rolls
         * @param number of dice sides
         * @return distribution of the dice roll
         */
        friend auto roll(
            const random_variable_decomposition& num_rolls, 
            const random_variable_decomposition& num_sides)
        {
            if (num_rolls.vars_.size() > 1 || num_sides.vars_.size()  > 1)
                throw std::runtime_error(
                    "Variable names are not supported in roll operator");
            return random_variable_decomposition(
                roll(num_rolls.vars_.front(), num_sides.vars_.front()));
        }

        /** Compute expected value of this random variable.
         * @return expected value of this variable
         */
        auto expected_value() const
        {
            auto var = to_random_variable();
            return var.expected_value();
        }

        /** Compute variance of this random variable.
         * @return variance of this variable
         */
        auto variance() const
        {
            auto var = to_random_variable();
            return var.variance();
        }

        /** Compute standard deviation of this random variable.
         * @return standard deviation
         */
        auto deviation() const
        {
            auto var = to_random_variable();
            return var.deviation();
        }

        /** Compute quantile of this random variable.
         * @param requested probability
         * @return quantile
         */
        auto quantile(ProbabilityType prob) const
        {
            auto var = to_random_variable();
            return var.quantile(prob);
        }

        /** Compute function of 2 random variables: A and B.
         * Variables does not need to be independent. 
         * @param other random variable B
         * @param combination function of 2 independent random variables
         * @return random variable that is a function of A and B
         */
        template<typename VarCombFunc>
        auto combine(
            const random_variable_decomposition& other, 
            VarCombFunc combination) const
        {
            random_variable_decomposition result;

            // compute union of the deps_ sets
            result.deps_ = sorted_union(deps_, other.deps_);

            // compute number of values for the deps_ vector
            std::size_t num_values = 1;
            for (auto&& var : result.deps_)
            {
                num_values *= var->probability().size();
            }

            // determine which dependencies are in A and which are in B
            const std::uint8_t A = 1;
            const std::uint8_t B = 2;
            std::vector<std::uint8_t> is_in;
            auto left = deps_.begin();
            auto right = other.deps_.begin();
            for (;;)
            {
                if (left == deps_.end() && right == other.deps_.end())
                {
                    break;
                }
                else if (left == deps_.end())
                {
                    is_in.push_back(B);
                    ++right;
                }
                else if (right == other.deps_.end())
                {
                    is_in.push_back(A);
                    ++left;
                }
                else if (*left < *right)
                {
                    is_in.push_back(A);
                    ++left;
                }
                else if (*right < *left)
                {
                    is_in.push_back(B);
                    ++right;
                }
                else // *right == *left 
                {
                    is_in.push_back(A | B);
                    ++left;
                    ++right;
                }
            }

            assert(is_in.size() == result.deps_.size());

            // compute the conditional random variables
            for (std::size_t i = 0; i < num_values; ++i)
            {
                std::size_t index_a = 0;
                std::size_t index_b = 0;

                std::size_t index_result = i;
                std::size_t size_a = 1;
                std::size_t size_b = 1;
                for (std::size_t j = 0; j < result.deps_.size(); ++j)
                {
                    auto&& var = result.deps_[j];
                    auto var_values_count = var->probability().size();
                    if ((is_in[j] & A) != 0)
                    {
                        index_a += (index_result % var_values_count) * size_a;
                        size_a *= var_values_count;
                    }
                    if ((is_in[j] & B) != 0)
                    {
                        index_b += (index_result % var_values_count) * size_b;
                        size_b *= var_values_count;
                    }
                    index_result /= var_values_count;
                }

                result.vars_.push_back(
                    combination(vars_[index_a], other.vars_[index_b]));
            }

            return result;
        }

        /** Convert this decomposition to basic random variable.
         * Notice, by performing this operation, we lose information 
         * about (in)dependence.
         * @return plain random variable
         */
        var_type to_random_variable() const 
        {
            var_type result;

            // if there are no dependent variables
            if (deps_.empty())
            {
                assert(vars_.size() <= 1);
                if (vars_.size() == 1)
                    return vars_[0];
                return result;
            }

            // store variables distributions in an array so we can access ith 
            // element faster
            std::vector<
                std::vector<
                    std::pair<value_type, ProbabilityType>>> vars;
            for (auto&& var : deps_)
            {
                vars.push_back(
                    std::vector<std::pair<value_type, ProbabilityType>>(
                        var->probability().begin(),
                        var->probability().end()
                    ));
            }
 
            for (std::size_t i = 0; i < vars_.size(); ++i)
            {
                // compute P(X = x)
                ProbabilityType prob = 1;
                std::size_t hash = i;
                for (auto&& value : vars)
                {
                    auto index = hash % value.size();
                    prob *= value[index].second;

                    hash /= value.size();
                }

                // compute P(A | X = x) * P(X = x)
                for (auto&& pair : vars_[i].probability())
                {
                    auto resp = result.probability_.insert(std::make_pair(
                        pair.first,
                        pair.second * prob
                    ));
                    if (!resp.second)
                    {
                        resp.first->second += pair.second * prob;
                    }
                }
            }
            return result;
        }

        /** Map value -> probability of this random variable.
         * @return probability map
         */
        auto probability() const
        {
            auto var = to_random_variable();
            return var.probability();
        }

        /** Check whether this decomposition has dependencies on other random 
         * variables.
         * @return true iff it has at least 1 dependency
         */
        bool has_dependencies() const 
        {
            return !deps_.empty();
        }

        /** Compute the decomposition of this random variable.
         *  It is only valid if there are no dependencies.
         */
        void compute_decomposition()
        {
            assert(!has_dependencies());
            assert(vars_.size() == 1);

            deps_.emplace_back(std::make_shared<var_type>(vars_.front()));
            vars_.clear();
            for (auto&& pair : deps_.front()->probability())
            {
                vars_.emplace_back(constant_tag{}, pair.first);
            }
        }

    private:
        /** Set of variables on which this random variable depends.
         * It is kept sorted by the pointer value. This simplifies the
         * combination algorithm as we can make some assumptions in the
         * variable index computation.
         */
        std::vector<std::shared_ptr<var_type>> deps_;

        /** List of conditional random variables.
         * 
         * Each item on this list is a r.v.: `A | X = x` where X is a random
         * vector deps_ and x is a value of this random vector.
         * 
         * `x` is defined implicitly by the index. For example, consider 
         * variables deps_ = (X, Y) whose values are 1 or 2. Order of variables
         * in this list is as follows:
         * -# A | X = 1, Y = 1 
         * -# A | X = 2, Y = 1 
         * -# A | X = 1, Y = 2 
         * -# A | X = 2, Y = 2
         */
        std::vector<var_type> vars_;
    };

    /** Compute maximum of 2 random variables.
     * Variables does not need to be independent.
     * @param first random variable
     * @param second random variable
     * @return maximum of the 2 random variables
     */
    template<typename ValueType, typename ProbabilityType>
    auto max(
        const random_variable_decomposition<ValueType, ProbabilityType>& a,
        const random_variable_decomposition<ValueType, ProbabilityType>& b)
    {
        return a.combine(b, [](auto&& var_a, auto&& var_b) 
        {
            return max(var_a, var_b);
        });
    }
    
    /** Compute minimum of 2 random variables.
     * Variables does not need to be independent.
     * @param first random variable
     * @param second random variable
     * @return minimum of the 2 random variables
     */
    template<typename ValueType, typename ProbabilityType>
    auto min(
        const random_variable_decomposition<ValueType, ProbabilityType>& a,
        const random_variable_decomposition<ValueType, ProbabilityType>& b)
    {
        return a.combine(b, [](auto&& var_a, auto&& var_b) 
        {
            return min(var_a, var_b);
        });
    }
}

#endif // DICE_RANDOM_VARIABLE_DECOMPOSITION_HPP_