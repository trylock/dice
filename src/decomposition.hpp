#ifndef DICE_DECOMPOSITION_HPP_
#define DICE_DECOMPOSITION_HPP_

#include <memory>
#include <vector>
#include <cassert>
#include <algorithm>

#include "utils.hpp"
#include "random_variable.hpp"

namespace dice
{
    template<typename ValueType, typename ProbabilityType>
    class decomposition_iterator;

    /** Object representing a decomposition of a function of random variables.
     * 
     * It uses the Law of total probability to decompose varibales to 
     * conditional variables which are then treated as independent.
     * 
     * Variables are organized in a tree structure. Only leaf nodes are stored 
     * in the vars_ list. Inner nodes are implicitly given by the deps_ list.
     * Each variable in the deps_ list corresponds to a level of this tree.
     * 
     * An inner node in this tree is a random variable. It has children 
     * corresponding to its values. For example: a random variable whose values
     * are 1 or 2 whould have 2 children labeled 1 and 2. A leaf node is 
     * a random variable without children nodes.
     */
    template<typename ValueType, typename ProbabilityType>
    class decomposition
    {
        friend class decomposition_iterator<ValueType, ProbabilityType>;
    public:
        using var_type = random_variable<ValueType, ProbabilityType>;
        using value_type = ValueType;
        using probability_type = ProbabilityType;
        using probability_iterator = 
            decomposition_iterator<ValueType, ProbabilityType>;

        decomposition() {}

        explicit decomposition(const var_type& variable)
        {
            vars_.push_back(variable);
        }

        explicit decomposition(var_type&& variable)
        {
            vars_.push_back(std::move(variable));
        }
        
        // Random variable constructors

        decomposition(
            std::vector<std::pair<value_type, std::size_t>>&& list)
            : decomposition(var_type(std::move(list)))
        {
        }

        decomposition(bernoulli_tag, ProbabilityType success)
            : decomposition(var_type(bernoulli_tag{}, success))
        {
        }
        
        decomposition(constant_tag, value_type value)
            : decomposition(var_type(constant_tag{}, value))
        {
        }
        
        // allow copy
        decomposition(const decomposition&) = default;
        decomposition& operator=(const decomposition&) = default;

        // allow move
        decomposition(decomposition&&) = default;
        decomposition& operator=(decomposition&&) = default;

        /** Compute sum of 2 random variables.
         * Random variables don't need to be indendent.
         * @param right hand side of the operator
         * @return distribution of the sum
         */
        auto operator+(const decomposition& other) const
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
        auto operator-(const decomposition& other) const
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
        auto operator*(const decomposition& other) const
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
        auto operator/(const decomposition& other) const
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
            decomposition result;
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
        auto less_than(const decomposition& other) const 
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
            const decomposition& other) const 
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
        auto equal(const decomposition& other) const 
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
        auto not_equal(const decomposition& other) const 
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
        auto greater_than(const decomposition& other) const 
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
            const decomposition& other) const 
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
            decomposition result;
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
            const decomposition& num_rolls, 
            const decomposition& num_sides)
        {
            return num_rolls.combine(num_sides, [](auto&& var_a, auto&& var_b)
            {
                // this is a different roll function (on random_variable types)
                // operands are independent random variables
                return roll(var_a, var_b);
            });
        }

        /** Compute expected value of this random variable.
         * @return expected value of this variable
         */
        auto expected_value() const
        {
            ProbabilityType expectation = 0;
            for (auto it = begin(); it != end(); ++it)
            {
                auto value = static_cast<probability_type>(it->first);
                expectation += value * it->second;
            }
            return expectation;
        }

        /** Compute variance of this random variable.
         * @return variance of this variable
         */
        auto variance() const
        {
            ProbabilityType sum_sq = 0;
            ProbabilityType sum = 0;
            for (auto it = begin(); it != end(); ++it)
            {
                auto value = static_cast<probability_type>(it->first);
                sum_sq += value * value * it->second;
                sum += value * it->second;
            }
            return sum_sq - sum * sum;
        }

        /** Compute standard deviation of this random variable.
         * @return standard deviation
         */
        auto deviation() const
        {
            return std::sqrt(variance());
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
            const decomposition& other, 
            VarCombFunc combination) const
        {
            decomposition result;

            // compute union of the deps_ sets
            result.deps_ = sorted_union(deps_, other.deps_);

            // compute number of values for the deps_ vector
            std::size_t num_values = 1;
            for (auto&& var : result.deps_)
            {
                num_values *= var->size();
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

                // find corresponding variables in both trees
                std::size_t index_result = i;
                std::size_t size_a = 1;
                std::size_t size_b = 1;
                for (std::size_t j = 0; j < result.deps_.size(); ++j)
                {
                    auto&& var = result.deps_[j];
                    auto var_values_count = var->size();
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

                // combine them
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
            for (auto it = begin(); it != end(); ++it)
            {
                result.add_probability(it->first, it->second);
            }
            return result;
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
         * Random variables in leafs (vars_ list) are made constants (making 
         * them independent) at the cost of adding new dependencies and thus 
         * increasing the size.
         * @return new decomposition
         */
        auto compute_decomposition() const
        {
            decomposition result;
            result.deps_ = deps_;

            std::vector<
                typename std::unordered_map<
                    ValueType, ProbabilityType>::const_iterator> state;

            // add new dependencies
            for (auto&& var : vars_)
            {
                if (!var.is_constant())
                {
                    result.deps_.push_back(make_variable_ptr(var));
                }
                state.push_back(var.begin());
            }

            // calculate new size
            std::size_t num_values = 1;
            for (auto&& dep : result.deps_)
            {
                num_values *= dep->size();
            }
            
            // add values
            for (std::size_t i = 0; i < num_values / state.size(); ++i)
            {
                for (std::size_t j = 0; j < state.size(); ++j)
                { 
                    result.vars_.emplace_back(constant_tag{}, state[j]->first);
                }

                // update state
                for (std::size_t j = 0; j < state.size(); ++j)
                {
                    ++state[j];
                    if (state[j] != vars_[j].end())
                    {
                        break;
                    }
                    state[j] = vars_[j].begin();
                }
            }
            return result;
        }

        /** Check whether this is exactly equal to other decomposition.
         * Note: this test is exact and expensive. It is mainly provided so
         *       we can use this type as a value in dice expressions.
         * @param other random variable
         * @return true iff the values are exactly equal
         */
        bool operator==(const decomposition& other) const
        {
            return deps_ == other.deps_ && vars_ == other.vars_;
        }

        bool operator!=(const decomposition& other) const
        {
            return deps_ != other.deps_ || vars_ != other.vars_; 
        }

        auto begin() const
        {
            return probability_iterator{ this, false };
        }

        auto end() const
        {
            return probability_iterator{ this, true };
        }

        // for debugging only
        auto& variables_internal() { return vars_; }
        auto& dependencies_internal() { return deps_; }
    private:
        struct var_ptr
        {
            using value_type = std::pair<std::size_t, var_type>;
            using pointer_type = std::shared_ptr<value_type>;

            var_ptr() : data_(nullptr) {}   
            var_ptr(var_type&& var) 
                : data_(
                    std::make_shared<value_type>(value_type{ 
                        counter_++, 
                        std::move(var) 
                    })) {}

            // allow copy
            var_ptr(const var_ptr&) = default;
            var_ptr& operator=(const var_ptr&) = default;

            // allow move
            var_ptr(var_ptr&&) = default;
            var_ptr& operator=(var_ptr&&) = default;

            // data getters
            var_type& variable()
            {
                assert(data_ != nullptr);
                return data_->second;
            }

            const var_type& variable() const
            {
                assert(data_ != nullptr);
                return data_->second;
            }

            std::size_t id() const
            {
                return data_ == nullptr ? 0 : data_->first;
            }

            const var_type* get() const
            {
                return &variable();
            }

            // pointer like access to variable
            var_type* operator->()
            {
                return &variable();
            }
            
            const var_type* operator->() const
            {
                return &variable();
            }

            // comparison
            bool operator<(const var_ptr& other) const
            {
                return id() < other.id();
            }

            bool operator==(const var_ptr& other) const
            {
                return id() == other.id();
            }

            bool operator!=(const var_ptr& other) const
            {
                return id() != other.id();
            }
        private:
            pointer_type data_;
            static std::size_t counter_;
        };

        var_ptr make_variable_ptr(var_type var) const
        {
            return var_ptr{ std::move(var) };
        }

        /** Set of variables on which this random variable depends.
         * It is kept sorted by the time it was created. This simplifies the 
         * combination algorithm as we can make some assumptions in the
         * variable index computation.
         */
        std::vector<var_ptr> deps_;

        /** List of conditional random variables.
         * 
         * Each item in this list is a r.v.: `A | X = x` where X is a random
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

    template<typename T, typename U>
    std::size_t decomposition<T, U>::var_ptr::counter_ = 1;

    /** Decompositon value iterator.
     * It will iterate through pairs (value, probability). One value can be in
     * multiple pairs. The probabilities sum up to 1.
     */
    template<typename ValueType, typename ProbabilityType>
    class decomposition_iterator
    {
    public:
        using decomposition_type = decomposition<ValueType, ProbabilityType>;
        using value_type = std::pair<ValueType, ProbabilityType>;

        explicit decomposition_iterator(
            const decomposition_type* decomp, 
            bool is_end = false)
            : decomposition_(decomp)
        {
            if (is_end)
            {
                leaf_it_ = leaf_end();
                return;
            }

            // initialize inner node iterators
            for (auto&& dep : decomposition_->deps_)
            {
                inner_it_.push_back(dep->begin());
            }

            // initialzie leaf and value iterators
            leaf_it_ = decomposition_->vars_.begin();
            if (leaf_it_ != leaf_end())
            {
                value_it_ = leaf_it_->begin();
            }

            // precompute first value
            precompute_value();
        }

        /** Move to the next value.
         * Next value is precomputed.
         * @return this iterator
         */
        decomposition_iterator& operator++()
        {
            // move to the next value in current leaf node
            ++value_it_;
            if (value_it_ == leaf_it_->end())
            {
                // move to the next leaf node
                ++leaf_it_;
                if (leaf_it_ != leaf_end())
                {
                    value_it_ = leaf_it_->begin();
                }

                // move inner node iterators
                for (std::size_t i = 0; i < inner_it_.size(); ++i)
                {
                    ++inner_it_[i];
                    if (inner_it_[i] != inner_end(i))
                    {
                        break;
                    }
                    inner_it_[i] = inner_begin(i);
                }
            }

            // precompute the value
            precompute_value();
            return *this;
        }

        /** Access current value.
         * @return pointer to current value
         */
        value_type* operator->()
        {
            return &current_value_;
        }

        /** Compare 2 iterators.
         * @param other iterator
         * @return true iff both point at the same value in the same variable
         */
        bool operator==(const decomposition_iterator& other) const
        {
            if (leaf_it_ == other.leaf_it_)
            {
                if (leaf_it_ == leaf_end())
                    return true;
                return value_it_ == other.value_it_;
            }
            return false;
        }

        bool operator!=(const decomposition_iterator& other) const 
        {
            return !operator==(other);
        }
    private:
        using rand_var = random_variable<ValueType, ProbabilityType>;
        using var_iterator = typename std::vector<rand_var>::const_iterator;
        using value_iterator = 
            typename std::unordered_map<ValueType, ProbabilityType>::const_iterator;

        // current decomposition object pointer
        const decomposition_type* decomposition_;
        // inner node value iterators
        std::vector<value_iterator> inner_it_;
        // leaf node variable iterator
        var_iterator leaf_it_;
        // value in this variable
        value_iterator value_it_;
        // current value
        value_type current_value_;

        auto inner_end(std::size_t index) const
        {
            return decomposition_->deps_[index]->end();
        }
        
        auto inner_begin(std::size_t index) const
        {
            return decomposition_->deps_[index]->begin();
        }

        auto leaf_end() const
        {
            return decomposition_->vars_.cend();
        }

        void precompute_value()
        {
            if (leaf_it_ == leaf_end())
                return;

            current_value_ = *value_it_;
            for (auto&& it : inner_it_)
            {
                current_value_.second *= it->second;
            }
        }
    };

    /** Compute maximum of 2 random variables.
     * Variables does not need to be independent.
     * @param first random variable
     * @param second random variable
     * @return maximum of the 2 random variables
     */
    template<typename ValueType, typename ProbabilityType>
    auto max(
        const decomposition<ValueType, ProbabilityType>& a,
        const decomposition<ValueType, ProbabilityType>& b)
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
        const decomposition<ValueType, ProbabilityType>& a,
        const decomposition<ValueType, ProbabilityType>& b)
    {
        return a.combine(b, [](auto&& var_a, auto&& var_b) 
        {
            return min(var_a, var_b);
        });
    }
}

#endif // DICE_DECOMPOSITION_HPP_