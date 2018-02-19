#ifndef DICE_RANDOM_VARIABLE_HPP_
#define DICE_RANDOM_VARIABLE_HPP_

#include <tuple>
#include <cassert>
#include <cmath>
#include <limits>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>

#ifdef min
#undef min
#endif // min

#ifdef max
#undef max
#endif // max

namespace dice 
{
    class bernoulli_tag{};
    class constant_tag{};

    template<typename ValueType, typename ProbabilityType>
    class decomposition;

    /** @brief Discrete random variable
     * 
     * @tparam ValueType type of a value of this variable. Requirements: 
     *                      - std::numeric_limits specialization
     *                      - std::hash specialization
     *                      - std::max, std::min specialization
     *                      - convertible to ProbabilityType
     * @tparam ProbabilityType type of probability (rational number in [0, 1])
     *
     * The ValueType is used as a key in a hash table. ProbabilityType is a 
     * value in this hash table. Probabilities of values sum up to 1.
     * 
     * This type is immutable. All public methods and functions which modify
     * a variable return a new random variable.
     */
    template<typename ValueType, typename ProbabilityType>
    class random_variable 
    {
        friend class decomposition<ValueType, ProbabilityType>;
    public:
        using value_type = ValueType;
        using probability_type = ProbabilityType;
        using frequency_list = std::vector<std::pair<value_type, std::size_t>>;
        using probability_list = std::vector<
            std::pair<value_type, probability_type>>;

        /** @brief Create an impossible event. */
        random_variable() {}

        /** @brief Create a constant.
         * @param value of the constant
         */
        random_variable(constant_tag, value_type value) : 
            probability_({ std::make_pair(value, 1.0) }) {}

        /** @brief Create a bernoulli distribution.
         * @param success_prob probability of success
         *        If this is 0 or 1, the variable will be constant. 
         */
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

        /** @brief Compute probabilities from list of value frequencies.
         * @param list of (value, frequency) pairs (values can repeat)
         */
        explicit random_variable(const frequency_list& list)
        {
            probability_type sum = 0;
            for (auto&& item : list)
                sum += item.second;

            for (auto&& item : list)
            {
                if (item.second == 0)
                    continue;
                
                add_probability(item.first, item.second / sum);
            }
        }

        ~random_variable() = default;

        // allow copy
        random_variable(const random_variable&) = default;
        random_variable& operator=(const random_variable&) = default;

        // allow move
        random_variable(random_variable&&) = default;
        random_variable& operator=(random_variable&&) = default;

        /** @brief Check whether this is a constant
         * 
         * @return true IFF there is only 1 value in its range
         */
        bool is_constant() const 
        {
            return probability_.size() == 1;
        }

        /** @brief Find maximal value in the variable's range.
         *
         * @return maximal value in the range or minimal value of the 
         *         value_type if this is an impossible event
         */
        auto max_value() const
        {
            auto value = std::numeric_limits<value_type>::lowest();
            for (auto&& pair : probability_)
            {
                value = std::max(value, pair.first);
            }
            return value;
        }

        /** @brief Find minimal value in the variable's range.
         *
         * @return minimal value in the range or maximal value of the 
         *         value_type if this is an impossible event
         */
        auto min_value() const
        {
            auto value = std::numeric_limits<value_type>::max();
            for (auto&& pair : probability_)
            {
                value = std::min(value, pair.first);
            }
            return value;
        }

        /** @brief Compute expected value of this random variable.
         *
         * @return expected value of this variable
         */
        auto expected_value() const 
        {
            probability_type exp = 0;
            for (auto&& pair : probability_)
            {
                auto value = static_cast<probability_type>(pair.first);
                exp += value * pair.second;
            }
            return exp;
        }

        /** @brief Compute variance of this random variable.
         *
         * @return variance of this variable
         */
        auto variance() const 
        {
            probability_type sum_sq = 0;
            probability_type sum = 0;
            for (auto&& pair : probability_)
            {
                auto value = static_cast<probability_type>(pair.first);
                sum_sq += value * value * pair.second;
                sum += value * pair.second;
            }
            return sum_sq - sum * sum;
        }

        /** @brief Calculate standard deviation of this random variable.
         *
         * @return standard deviation (square root or variance)
         */
        auto deviation() const
        {
            return std::sqrt(variance());
        }

        /** @brief Compute quantile of this random variable.
         *
         * Def.: Quantile(p) = min{ x : P(X <= x) >= p} 
         * Note: complexity of this operation is linearithmic with the size of
         *       the variable as we have to sort the values.
         *       
         * @throws std::logic_error if this is an impossible event.
         *       
         * @param probability between 0 and 1
         * 
         * @return quantile
         */
        auto quantile(probability_type probability) const
        {
            if (probability_.empty())
                throw std::logic_error("Quantile is not defined.");

            // sort the values
            probability_list list{ probability_.begin(), probability_.end() };
            std::sort(list.begin(), list.end(), [](auto&& a, auto&& b)
            {
                return a.first < b.first;
            });

            // compute the quantile
            value_type result = list.front().first;
            probability_type prob_sum = 0;
            for (auto&& value : list)
            {
                if (prob_sum >= probability)
                {
                    break;
                }
                prob_sum += value.second;
                result = value.first;
            }
            return result;
        }

        /** @brief Return first value s.t. P(X <= value) >= prob.
         *
         * Unlike the quantile method, values are not copied nor sorted.
         * 
         * @param probability (a random number between 0 and 1)
         * 
         * @return value
         */
        auto random_value(probability_type probability) const
        {
            probability_type sum = 0;
            for (auto&& pair : probability_)
            {
                if (sum + pair.second >= probability)
                {
                    return pair.first;
                }
                sum += pair.second;
            }
            return probability_.end()->first;
        }

        /** @brief Calculate indicator that X (this r.v.) is in given interval
         *
         * @param lower_bound of the interval
         * @param upper_bound of the interval
         * 
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

        /** @brief Compute distribution of X + Y (X is this random variable).
         *
         * X and Y are assumed to be independent.
         * 
         * @param other random variable Y (independent of X)
         * 
         * @return distribution of X + Y
         */
        auto operator+(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return a + b;
            });
        }

        /** @brief Compute distribution of X - Y (X is this random variable).
         *
         * X and Y are assumed to be independent.
         * 
         * @param other random variable Y (independent of X)
         * 
         * @return distribution of X - Y
         */
        auto operator-(const random_variable& other) const
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return a - b;
            });
        }

        /** @brief Compute distribution of X * Y (X is this random variable).
         *
         * X and Y are assumed to be independent.
         * 
         * @param other random variable Y (independent of X)
         * 
         * @return distribution of X * Y
         */
        auto operator*(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return a * b;
            });
        }

        /** @brief Compute distribution of integer division X / Y.
         *
         * X is this random variable.
         * X and Y are assumed to be independent.
         * 
         * @param other random variable Y (independent of X)
         * @return distribution of X / Y
         */
        auto operator/(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b)
            {
                return a / b; 
            });
        }

        /** @brief Compute indicator of X < Y (X is this random variable).
         *
         * X and Y are assumed to be indepedent.
         * 
         * @param other random variable Y (independent of X)
         * 
         * @return indicator of X < Y 
         *         (i.e. a random variable with a bernoulli distribution)
         */
        auto less_than(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a < b);
            });
        }

        /** @brief Compute indicator of X <= Y (X is this random variable).
         *
         * X and Y are assumed to be indepedent.
         * 
         * @param other random variable Y (independent of X)
         * 
         * @return indicator of X <= Y 
         *         (i.e. a random variable with a bernoulli distribution)
         */
        auto less_than_or_equal(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a <= b);
            });
        }

        /** @brief Compute indicator of X = Y (X is this random variable).
         *
         * X and Y are assumed to be indepedent.
         * 
         * @param other random variable Y (independent of X)
         * 
         * @return indicator of X = Y 
         *         (i.e. a random variable with a bernoulli distribution)
         */
        auto equal(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a == b);
            });
        }
        
        /** @brief Compute indicator of X != Y (X is this random variable).
         *
         * X and Y are assumed to be indepedent.
         * 
         * @param other random variable Y (independent of X)
         * 
         * @return indicator of X != Y 
         *         (i.e. a random variable with a bernoulli distribution)
         */
        auto not_equal(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a != b);
            });
        }
        
        /** @brief Compute indicator of X > Y (X is this random variable).
         *
         * X and Y are assumed to be indepedent.
         * 
         * @param other random variable Y (independent of X)
         * 
         * @return indicator of X > Y 
         *         (i.e. a random variable with a bernoulli distribution)
         */
        auto greater_than(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a > b);
            });
        }
        
        /** @brief Compute indicator of X >= Y (X is this random variable).
         *
         * X and Y are assumed to be indepedent.
         * 
         * @param other random variable Y (independent of X)
         * 
         * @return indicator of X >= Y 
         *         (i.e. a random variable with a bernoulli distribution)
         */
        auto greater_than_or_equal(const random_variable& other) const 
        {
            return combine(other, [](auto&& a, auto&& b) 
            {
                return static_cast<value_type>(a >= b);
            });
        }

        /** @brief Compute negation of this random variable (-X)
         *
         * @return a random variable -X (X is this random variable)
         */
        auto operator-() const 
        {
            random_variable result;
            for (auto&& pair : probability_)
            {
                result.probability_.insert(
                    std::make_pair(-pair.first, pair.second));
            }
            return result;
        }

        /** @brief Compute restriction of the range of this random variable.
         *
         * Values for which given predicate returns false won't be included.
         * Probabilities will be modified so that they sum up to 1.
         * 
         * @param include predicate (bool function)
         *        Returns true IFF given element should be included
         *        
         * @return a new random variable with restricted range
         */
        template<typename Predicate>
        auto restrict(Predicate include) const
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

        /** @brief Create a random variable Z = XdY.
         *
         * X times roll a Y sided dice.
         * Assumes X, Y are independent random variables.
         * 
         * This funciton implements a simple dynamic programming algorithm.
         * The probability that we roll k with X throws of a Y sided die is
         * the probability that we roll k - n with X - 1 throws of a Y sided
         * die and then roll n on the last die for n = 1 to Y.
         * 
         * @param num_dice number of dice X (independent of Y)
         *        Each value has to be a positive integer.
         * @param num_faces number of faces of each die Y (independent of X)
         *        Each value has to be a positive integer.
         *        
         * @return distribution of XdY
         * 
         * @throws std::invalid_argument if X or Y contain a 0 value.
         */
        friend auto roll(
            const random_variable& num_dice, 
            const random_variable& num_faces)
        {
            // If there are no dice or dice sizes, 
            // then this is an impossible event
            if (num_dice.probability_.empty() || 
                num_faces.probability_.empty())
            {
                return random_variable{};
            }

            // Throw an exception if the number of dice or
            // number of dice faces is not positive.
            // note: Use the restrict method to restrict variable's 
            //       range to positive integers.
            for (auto&& pair : num_dice.probability_)
            {
                if (pair.first <= 0)
                {
                    throw std::invalid_argument(
                        "Number of dice has to be a positive integer.");
                }
            }

            for (auto&& pair : num_faces.probability_)
            {
                if (pair.first <= 0)
                {
                    throw std::invalid_argument(
                        "Number of dice faces has to be a positive integer.");
                }
            }

            // find maximal number of dice and faces
            auto max_dice = num_dice.max_value();
    
            // compute distribution for each possible number of faces
            random_variable dist;
            for (auto&& pair : num_faces.probability_)
            {
                auto faces_count = pair.first;
                auto faces_prob = pair.second;
                auto base_prob = 1 / static_cast<probability_type>(faces_count);
                
                // save the probability of 1 roll
                auto one_roll = num_dice.probability_.find(1);
                if (one_roll != num_dice.probability_.end())
                {
                    auto rolls_prob = one_roll->second;
                    auto prob = base_prob * faces_prob * rolls_prob;
                    for (value_type i = 1; i <= faces_count; ++i)
                    {
                        dist.add_probability(i, prob);
                    }
                }

                // Prefix sum of probability:
                // P(XdY = k | X = dice_count, Y = faces_count)
                std::vector<probability_type> probability(
                    faces_count * max_dice + 1, 0);
                
                // base case: roll only 1 die
                for (value_type i = 1; i <= faces_count; ++i)
                {
                    probability[i] = base_prob;
                }

                // Roll `dice_count` dice given the result of 
                // `dice_count - 1` dice
                for (value_type dice_count = 2; 
                    dice_count <= max_dice; 
                    ++dice_count)
                {
                    // compute the prefix sum of the probability array
                    for (value_type i = 2; i <= faces_count * dice_count; ++i)
                    {
                        probability[i] = probability[i - 1] + probability[i];
                    }

                    // For computation of the probability of the sum of i
                    // we only need values j < i. By iterating backwards we 
                    // don't overwrite those values.
                    for (auto i = faces_count * dice_count; i >= dice_count; --i)
                    {
                        // compute the probability of the sum of i
                        value_type j = std::max(i - faces_count, 1);
                        auto prob_i = probability[i - 1] - probability[j - 1];
                        prob_i *= base_prob;

                        // We will break the invariant that the probability 
                        // array is a prefix sum of the probabilities but it
                        // will be restored in the next iteration.
                        probability[i] = prob_i;
                    }

                    // zero out probabilities of lower values
                    for (value_type i = 1; i < dice_count; ++i)
                    {
                        probability[i] = 0;
                    }

                    // check whether dice_count is a valid number of dice
                    auto rolls_it = num_dice.probability_.find(dice_count);
                    if (rolls_it == num_dice.probability_.end())
                    {
                        continue;
                    }

                    // save the probability 
                    auto rolls_prob = rolls_it->second;
                    for (auto i = dice_count; i <= faces_count * dice_count; ++i)
                    {
                        dist.add_probability(
                            i, 
                            probability[i] * rolls_prob * faces_prob
                        );
                    }
                }
            }
            return dist;
        }

        /** @brief Create a random variable that is a function
         *         of this variable X and the other variable Y.
         *
         * @tparam CombinationFunction 
         *            It takes value from X (ValueType), value from Y (ValueType) 
         *            and returns their combination (ValueType).
         *        
         * @param other random variable Y (independent of X)
         * @param combination function of X and Y
         * 
         * @return a new random variable that is a function of X and Y
         */
        template<typename CombinationFunction>
        auto combine(
            const random_variable& other, 
            CombinationFunction combination) const
        {
            random_variable dist;
            for (auto&& pair_a : probability_)
            {
                for (auto&& pair_b : other.probability_)
                {
                    auto value = combination(pair_a.first, pair_b.first);
                    auto probability = pair_b.second * pair_a.second;
                    dist.add_probability(value, probability);
                }
            }
            return dist;
        }

        /** @brief Find the probability of given value.
         *
         * @param value whose probability you want to know.
         * 
         * @return probability of the value.
         *         0 if it's not in the varaible's range
         */
        auto probability(const value_type& value) const
        {
            auto it = probability_.find(value);
            if (it == probability_.end())
                return probability_type{ 0 };
            return it->second;
        }

        /** @brief Return number of values in variable's range. 
         *
         * Number of values with non-zero probability.
         * This is also the std::distance(begin(), end()).
         * 
         * @return number of such values
         */
        auto size() const
        {
            return probability_.size();
        }

        /** @brief First iterator of the (value, probaiblity) pair collection.
         *
         * @return iterator pointing to the first value
         */
        auto begin() const
        {
            return probability_.begin();
        }

        /** @brief Last iterator of the (value, probability) pair collection.
         *
         * @return iterator pointing past the last value
         */
        auto end() const
        {
            return probability_.cend();
        }

        /** @brief Check whether there are any values with non-zero probability.
         *
         * @return true iff there is at least 1 value with non-zero probability
         */
        bool empty() const
        {
            return probability_.empty();
        }

        /** @brief Chek whether this variable is equal to some other variable.
         *
         * Note: this will use the == operator on the ProbabilityType.
         *       This test is expensive (linear with the variable size).
         * 
         * @param other random variable
         * 
         * @return true iff the variables are exactly equal
         */
        bool operator==(const random_variable& other) const
        {
            return probability_ == other.probability_;
        }

        bool operator!=(const random_variable& other) const
        {
            return probability_ != other.probability_;
        }
    private:
        std::unordered_map<value_type, probability_type> probability_;

        /** @brief Add probability to current porbability of given value.
         *
         * Note: caller guarantees that probabilities sum up to 1.
         * 
         * @param value
         * @param probability that will be added to current probability of value
         */
        void add_probability(value_type value, probability_type probability)
        {
            auto result = probability_.insert(
                std::make_pair(value, probability));
            auto iter = result.first;
            auto is_inserted = result.second;
            if (!is_inserted)
            {
                iter->second += probability;
            }
        }
    };

    /** @brief Calculate max(X, Y) for independent r.v. X and Y
     * 
     * @tparam T ValueType of random variable
     * @tparam U ProbabilityType of random variable
     * 
     * @param a first random variable X (independent of Y)
     * @param b second random variable Y (independent of X)
     * 
     * @return distribution of max(X, Y)
     */
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

    /** @brief Calculate min(X, Y) for independent r.v. X and Y
    *
    * @tparam T ValueType of random variable
    * @tparam U ProbabilityType of random variable
    *
    * @param a first random variable X (independent of Y)
    * @param b second random variable Y (independent of X)
    *
    * @return distribution of min(X, Y)
    */
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