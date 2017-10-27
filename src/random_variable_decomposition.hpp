#ifndef DICE_RANDOM_VARIABLE_DECOMPOSITION_HPP_
#define DICE_RANDOM_VARIABLE_DECOMPOSITION_HPP_

#include <vector>
#include <cassert>
#include <unordered_set>
#include <unordered_map>

#include "random_variable.hpp"

namespace dice
{
    // Decomposition should treat given variable as always independent.
    class independent_tag{};

    /** Decomposition should expect that given variable might depend
     * on some other variable when computing a combination of 2 variables.
     */
    class dependent_tag{};

    /** Object representing a decomposition of a function of random variables.
     * 
     * Denote X a random variable. Let us assume that X depends on random 
     * variables A1, ..., An. Then, from Law of total probability: 
     * `X = sum(X | A1 = a1, ..., An = an for all a1, ..., an)`
     * Because events `A1 = a1, ..., An = an` are disjoint and they form 
     * the whole sample space (we are considering all such ai's)
     * 
     * Purpose of this is that variables `X | A1 = a1, ..., An = an` might 
     * be independent.
     */
    template<typename ValueType, typename ProbabilityType>
    class random_variable_decomposition
    {
    public:
        using var_type = random_variable<ValueType, ProbabilityType>;
        using value_type = ValueType;

        random_variable_decomposition() {}

        random_variable_decomposition(independent_tag, var_type* variable)
        {
            assert(variable != nullptr);
            vars_.push_back(*variable);
        }

        random_variable_decomposition(dependent_tag, var_type* variable)
        {
            assert(variable != nullptr);
            deps_.push_back(variable);

            for (auto&& pair : variable->probability())
            {
                vars_.emplace_back(constant_tag{}, pair.first);
            }
        }

        /** Compute sum of 2 random variables.
         * Random variables don't need to be indendent.
         * @param right hand side of the operator
         * @return distribution of the sum
         */
        auto operator+(const random_variable_decomposition& other) const
        {
            return combine(other, [](auto&& value_a, auto&& value_b) 
            {
                return value_a + value_b;
            });
        }
        
        /** Subtract other random variable from this.
         * Random variables don't need to be indendent.
         * @param right hand side of the operator
         * @return result of the subtraction
         */
        auto operator-(const random_variable_decomposition& other) const
        {
            return combine(other, [](auto&& value_a, auto&& value_b) 
            {
                return value_a - value_b;
            });
        }
        
        /** Compute product of 2 random variables.
         * Random variables don't need to be indendent.
         * @param right hand side of the operator
         * @return result of the product
         */
        auto operator*(const random_variable_decomposition& other) const
        {
            return combine(other, [](auto&& value_a, auto&& value_b) 
            {
                return value_a * value_b;
            });
        }
        
        /** Divide this variable by other variable.
         * Random variables don't need to be indendent.
         * @param right hand side of the operator
         * @return result of the division
         */
        auto operator/(const random_variable_decomposition& other) const
        {
            return combine(other, [](auto&& value_a, auto&& value_b) 
            {
                return value_a / value_b;
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

        /** Compute indicator: X < Y (where X is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable Y
         * @return indicator of X < Y 
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto less_than(const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a < b);
            });
        }

        /** Compute indicator: X <= Y (where X is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable Y
         * @return indicator of X <= Y 
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto less_than_or_equal(
            const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a <= b);
            });
        }

        /** Compute indicator: X = Y (where X is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable Y
         * @return indicator of X = Y 
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto equal(const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a == b);
            });
        }

        /** Compute indicator: X != Y (where X is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable Y
         * @return indicator of X != Y 
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto not_equal(const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a != b);
            });
        }

        /** Compute indicator: X > Y (where X is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable Y
         * @return indicator of X > Y 
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto greater_than(const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a > b);
            });
        }
        
        /** Compute indicator: X >= Y (where X is this random variale).
         * Variables does not need to be indepedent.
         * @param other random variable Y
         * @return indicator of X >= Y 
         * (i.e. a random variable with a bernoulli distribution)
         */
        auto greater_than_or_equal(
            const random_variable_decomposition& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a >= b);
            });
        }

        /** Compute indicator: X in [A, B] (where X is this random variable).
         * The interval is closed.
         * @param lower bound of the interval A
         * @param upper bound of the interval B
         * @return indicator of X in [A, B]
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
            return roll(num_rolls.vars_.front(), num_sides.vars_.front());
        }

        /** Compute function of 2 random variables: X and Y.
         * Variables does not need to be independent. 
         * @param other random variable Y
         * @param combination function
         * @return random variable that is a function of X and Y
         */
        template<typename CombinationFunc>
        auto combine(
            const random_variable_decomposition& other, 
            CombinationFunc combination) const
        {
            random_variable_decomposition result;

            // compute union of the deps_ sets
            sorted_union(deps_, other.deps_, result.deps_);

            // compute number of values for the deps_ vector
            std::size_t num_values = 1;
            for (auto&& var : result.deps_)
            {
                num_values *= var->probability().size();
            }

            std::unordered_set<var_type*> in_a{ deps_.begin(), deps_.end() };
            std::unordered_set<var_type*> in_b{ 
                other.deps_.begin(), other.deps_.end() };

            // compute the conditional random variables
            for (std::size_t i = 0; i < num_values; ++i)
            {
                std::size_t index_a = 0;
                std::size_t index_b = 0;

                std::size_t hash = i;
                std::size_t size_a = 1;
                std::size_t size_b = 1;
                for (auto&& var : result.deps_)
                {
                    if (in_a.find(var) != in_a.end())
                    {
                        index_a += (hash % var->probability().size()) * size_a;
                        size_a *= var->probability().size();
                    }
                    if (in_b.find(var) != in_b.end())
                    {
                        index_b += (hash % var->probability().size()) * size_b;
                        size_b *= var->probability().size();
                    }
                    hash /= var->probability().size();
                }

                result.vars_.push_back(
                    vars_[index_a].combine(other.vars_[index_b], combination));
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

    private:
        /** Set of variables on which this random variable depends.
         * It is kept sorted by the pointer value. This simplifies the
         * combination algorithm as we can make some assumptions in the
         * variable index computation.
         * 
         * Note we store a pointer to each variable. Therefore, they have to
         * outlive this object.
         */
        std::vector<var_type*> deps_;

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

        /** Compute union of sorted lists A and B.
         * Both lists have to be sorted by given comparer.
         * @param sorted list A
         * @param sorted list B
         * @param list where the sorted union will be stored
         * @param comparer
         */
        template<typename T, typename Less = std::less<T>>
        static void sorted_union(
            const std::vector<T>& a, 
            const std::vector<T>& b,
            std::vector<T>& result,
            Less is_less = Less())
        {
            auto left = a.begin();
            auto right = b.begin();
            for (;;)
            {
                if (left == a.end() && right == b.end())
                {
                    break;
                }
                else if (right == b.end())
                {
                    result.push_back(*left++);
                }
                else if (left == a.end())
                {
                    result.push_back(*right++);
                }
                else if (is_less(*left, *right))
                {
                    result.push_back(*left++);
                }
                else if (is_less(*right, *left))
                {
                    result.push_back(*right++);
                }
                else // *left == *right
                {
                    result.push_back(*left);
                    ++left;
                    ++right;
                }
            }
        }
    };
}

#endif // DICE_RANDOM_VARIABLE_DECOMPOSITION_HPP_