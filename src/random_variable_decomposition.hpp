#ifndef DICE_RANDOM_VARIABLE_DECOMPOSITION_HPP_
#define DICE_RANDOM_VARIABLE_DECOMPOSITION_HPP_

#include <vector>
#include <cassert>
#include <unordered_set>
#include <unordered_map>

#include "random_variable.hpp"

namespace dice
{
    class independent_tag{};
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

        template<typename CombinationFunc>
        auto combine(
            const random_variable_decomposition& other, 
            CombinationFunc combination) const
        {
            random_variable_decomposition result;

            // compute union of the deps_ sets
            auto left = deps_.begin();
            auto right = other.deps_.begin();
            for (;;)
            {
                if (left == deps_.end() && right == other.deps_.end())
                {
                    break;
                }
                else if (right == other.deps_.end())
                {
                    result.deps_.push_back(*left++);
                }
                else if (left == deps_.end())
                {
                    result.deps_.push_back(*right++);
                }
                else if (*left < *right)
                {
                    result.deps_.push_back(*left++);
                }
                else if (*left == *right)
                {
                    result.deps_.push_back(*left);
                    ++left;
                    ++right;
                }
                else 
                {
                    result.deps_.push_back(*right++);
                }
            }

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
                std::size_t old_size = 1;
                for (auto&& var : result.deps_)
                {
                    if (in_a.find(var) != in_a.end())
                    {
                        index_a *= old_size;
                        index_a += hash % var->probability().size();
                    }
                    if (in_b.find(var) != in_b.end())
                    {
                        index_b *= old_size;
                        index_b += hash % var->probability().size();
                    }
                    hash /= var->probability().size();
                    old_size = var->probability().size();
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

            // store variables distributions in an array so we can acces ith 
            // element faster
            std::vector<
                std::vector<
                    std::pair<value_type, ProbabilityType>>> vars;
            for (auto&& var : deps_)
            {
                vars.push_back(std::vector<std::pair<value_type, ProbabilityType>>(
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
         * `x` is defined implicitly by the lexicographical order on (i, j) 
         * where i is index of a component of X and j is index of a value 
         * of this component.
         */
        std::vector<var_type> vars_;
    };
}

#endif // DICE_RANDOM_VARIABLE_DECOMPOSITION_HPP_