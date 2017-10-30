#ifndef DICE_RANDOM_VARIABLE_HPP_
#define DICE_RANDOM_VARIABLE_HPP_

#include <cassert>
#include <limits>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>

namespace dice 
{
    class bernoulli_tag{};
    class constant_tag{};

    template<typename ValueType, typename ProbabilityType>
    class random_variable_decomposition;

    // discrete random variable
    template<typename ValueType, typename ProbabilityType>
    class random_variable 
    {
        friend class random_variable_decomposition<ValueType, ProbabilityType>;
    public:
        using value_type = ValueType;
        using probability_type = ProbabilityType;
        using freq_list = std::vector<std::pair<value_type, std::size_t>>;

        // create an impossible event
        random_variable() {}

        // create a constant 
        random_variable(constant_tag, value_type value) : 
            probability_({ std::make_pair(value, 1.0) }) {}

        // create a bernoulli distribution
        random_variable(bernoulli_tag, probability_type success_prob) : 
            probability_({ 
                std::make_pair(0, 1 - success_prob), 
                std::make_pair(1, success_prob) 
            }) 
        {
            // make this a constant if the success probability is 1 or 0
            if (success_prob <= 0)
            {
                probability_.erase(probability_.find(1));
            }
            else if (success_prob >= 1)
            {
                probability_.erase(probability_.find(0));
            }
        }

        random_variable(freq_list&& list)
        {
            probability_type sum = 0;
            for (auto&& item : list)
                sum += item.second;

            for (auto&& item : list)
            {
                if (item.second == 0)
                    continue;

                probability_.insert(std::make_pair(
                    item.first,
                    item.second / sum
                ));
            }
        }

        // allow copy
        random_variable(const random_variable&) = default;
        random_variable& operator=(const random_variable&) = default;

        // allow move
        random_variable(random_variable&&) = default;
        random_variable& operator=(random_variable&&) = default;

        // true IFF there is only 1 value in its range
        bool is_constant() const 
        {
            return probability_.size() == 1;
        }

        // value of the variable (makes sense for constant variables only)
        value_type value() const 
        {
            assert(is_constant());
            return probability_.begin()->first;
        }

        /** Compute expected value of this random variable.
         * @return expected value of this variable
         */
        auto expected_value() const 
        {
            probability_type exp = 0;
            for (auto&& pair : probability_)
            {
                exp += pair.first * pair.second;
            }
            return exp;
        }

        /** Compute variance of this random variable.
         * @return variance of this variable
         */
        auto variance() const 
        {
            probability_type sum_sq = 0;
            probability_type sum = 0;
            for (auto&& pair : probability_)
            {
                sum_sq += pair.first * pair.first * pair.second;
                sum += pair.first * pair.second;
            }
            return sum_sq - sum * sum;
        }

        /** Calculate indicator that X (this r.v.) is in given interval
         * @param lower bound of the interval
         * @param upper bound of the interval
         * @return r.v. Y with a Bernoulli distribution 
         *         Y = 1 <=> X is in the [lower_bound, upper_bound] interval
         *         Y = 0 otherwise
         */
        template<typename T>
        random_variable in(const T& lower_bound, const T& upper_bound) const
        {
            probability_type success_prob = 0;
            for (auto&& pair : probability_)
            {
                if (lower_bound <= static_cast<T>(pair.first) && 
                    upper_bound >= static_cast<T>(pair.first))
                {
                    success_prob += pair.second;
                }
            }
            return random_variable{ bernoulli_tag{}, success_prob };
        }

        // immutable operations on random variables (assumes independence)
        random_variable operator+(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return a + b;
            });
        }

        random_variable operator-(const random_variable& other) const
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return a - b;
            });
        }

        random_variable operator*(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return a * b;
            });
        }

        random_variable operator/(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b)
            {
                return a / b; 
            });
        }

        random_variable less_than(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a < b);
            });
        }

        random_variable less_than_or_equal(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a <= b);
            });
        }

        random_variable equal(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a == b);
            });
        }
        
        random_variable not_equal(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a != b);
            });
        }
        
        random_variable greater_than(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a > b);
            });
        }
        
        random_variable greater_than_or_equal(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a >= b);
            });
        }

        random_variable operator-() const 
        {
            random_variable result;
            for (auto&& pair : probability_)
            {
                result.probability_.insert(std::make_pair(-pair.first, pair.second));
            }
            return result;
        }

        /** Compute restriction of the range of this random variable.
         * Values for which given predicate returns false won't be included.
         * Probabilities will be modified so that they sum up to 1.
         * @param include predicate 
         *        Returns true IFF given element should be included
         * @return a new random variable with restricted range
         */
        template<typename Predicate>
        random_variable restrict(Predicate include) const
        {
            probability_type prob_sum = 0;
            for (auto&& pair : probability_)
            {
                if (include(pair.first))
                {
                    prob_sum += pair.second;
                }
            }
            
            random_variable result;
            for (auto&& pair : probability_)
            {
                if (include(pair.first))
                {
                    // make sure probabilities sum up to 1
                    result.probability_.insert(std::make_pair(
                        pair.first,
                        pair.second / prob_sum 
                    ));
                }
            }
            return result;
        }

        /** 
         * Create a random variable Z = XdY.
         * X times roll a Y sided dice.
         * Assumes X, Y are independent random variables.
         * 
         * This funciton implements a simple dynamic programming algorithm.
         * The probability that we roll k with X throws of a Y sided die is
         * the probability that we roll k - n with X - 1 throws of a Y sided
         * die and then roll n on the last die for n = 1 to Y.
         * 
         * @param number of dice X (independent of Y)
         *        Each value has to be a positive integer
         * @param number of sides of each dice Y (independent of X)
         *        Each value has to be a positive integer
         * @return distribution of XdY
         */
        friend random_variable roll(
            const random_variable& num_dice, 
            const random_variable& num_sides)
        {
            // If there are no dice or dice sizes, 
            // then this is an impossible event
            if (num_dice.probability_.size() <= 0 || 
                num_sides.probability_.size() <= 0)
            {
                return random_variable{};
            }

            // Throw an exception if the number of dice or
            // number of dice sides is not positive.
            // note: Use the restrict method to restrict variable's 
            //       range to positive integers.
            for (auto&& pair : num_dice.probability_)
            {
                if (pair.first <= 0)
                {
                    throw std::runtime_error(
                        "Number of dice has to be a positive integer.");
                }
            }

            for (auto&& pair : num_sides.probability_)
            {
                if (pair.first <= 0)
                {
                    throw std::runtime_error(
                        "Number of dice sides has to be a positive integer.");
                }
            }

            // find maximal number of dice
            auto max_dice = num_dice.probability_.rbegin()->first;
    
            // compute distribution for each possible number of sides
            random_variable dist;
            for (auto&& pair : num_sides.probability_)
            {
                auto sides_count = pair.first;
                auto sides_prob = pair.second;

                // probability P(XdY = k | X = dice_count, Y = sides_count)
                std::vector<probability_type> probability(
                    sides_count * max_dice + 1, 0);
                for (value_type dice_count = 1; 
                    dice_count <= max_dice; 
                    ++dice_count)
                {
                    // iterate backwards so that we don't override 
                    // probability values that will be needed
                    for (value_type i = sides_count * dice_count; i > 0; --i)
                    {
                        // compute the probability of the sum of i
                        auto base_prob = 
                            1 / static_cast<probability_type>(sides_count);
                        auto prob_i = dice_count == 1 ? base_prob : 0;
                        value_type j = std::max(i - sides_count, 1);
                        for (; j < i; ++j)
                        {
                            prob_i += probability[j] * base_prob;
                        }
                        probability[i] = prob_i;
    
                        // save the probability
                        auto num_rolls = num_dice.probability_.find(dice_count);
                        if (num_rolls != num_dice.probability_.end())
                        {
                            auto rolls_prob = num_rolls->second;
                            auto prob = prob_i * sides_prob * rolls_prob;
                            // don't save impossible events
                            if (prob == 0) 
                                continue;
    
                            auto result = dist.probability_.insert(
                                std::make_pair(i, prob));
                            if (!result.second)
                            {
                                result.first->second += prob;
                            }
                        }
                    }
                }
            }
            return dist;
        }

        /** Create a random variable that is a function
         *  of this r.v. (X) and the other r.v. (Y)
         * @param other random variable Y (independent of X)
         * @param combination function of X and Y
         * @return a new random variable that is a function of X and Y
         */
        template<typename Func>
        random_variable combine(const random_variable& other, Func&& combination) const
        {
            random_variable dist;
            for (auto&& pair_a : probability_)
            {
                for (auto&& pair_b : other.probability_)
                {
                    auto value = combination(pair_a.first, pair_b.first);
                    auto probability = pair_b.second * pair_a.second;
                    auto result = dist.probability_.insert(std::make_pair(value, probability));
                    if (!result.second)
                    {
                        result.first->second += probability;
                    }
                }
            }
            return dist;
        }

        const std::map<value_type, probability_type>& probability() const 
        {
            return probability_; 
        }
    private:
        std::map<value_type, probability_type> probability_;
    };

    // Calculate max(X, Y) for independent r.v. X and Y
    template<typename T, typename U>
    random_variable<T, U> max(
        const random_variable<T, U>& a, 
        const random_variable<T, U>& b)
    {
        return a.combine(b, [](auto&& value_a, auto&& value_b) 
        {
            return std::max(value_a, value_b);
        });
    }
    
    // Calculate min(X, Y) for independent r.v. X and Y
    template<typename T, typename U>
    random_variable<T, U> min(
        const random_variable<T, U>& a, 
        const random_variable<T, U>& b)
    {
        return a.combine(b, [](auto&& value_a, auto&& value_b) 
        {
            return std::min(value_a, value_b);
        });
    }
}

#endif // DICE_RANDOM_VARIABLE_HPP_