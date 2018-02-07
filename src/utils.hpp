#ifndef DICE_UTILS_HPP_
#define DICE_UTILS_HPP_

#include <vector>
#include <functional>

namespace dice 
{
    /** @brief Compute union of sorted lists A and B.
     *
     * Both lists have to be sorted by given comparer.
     * 
     * @param a sorted list A
     * @param b sorted list B
     * @param is_less comparer (use operator < by default)
     * 
     * @return sorted union of lists A and B
     */
    template<typename T, typename Less = std::less<T>>
    std::vector<T> sorted_union(
        const std::vector<T>& a, 
        const std::vector<T>& b,
        Less is_less = Less())
    {
        std::vector<T> result;
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
        return result;
    }
    
    /** @brief Clamp a value to given range.
     *
     * @param value
     * @param lower bound of the range
     * @param upper bound of the range
     * 
     * @return lower if value < lower
     *         upper if value > upper
     *         value otherwise 
     */
    template<typename T>
    T clamp(const T& value, const T& lower, const T& upper)
    {
        if (value < lower)
            return lower;
        if (value > upper)
            return upper;
        return value;
    }
}

#endif // DICE_UTILS_HPP_