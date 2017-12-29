#ifndef DICE_SAFE_HPP_
#define DICE_SAFE_HPP_

#define NEEDS_NULLPTR_DEFINED 0
#include <SafeInt3.hpp>

#ifdef max
#undef max
#endif // max

#ifdef min
#undef min
#endif // min

namespace dice
{
    template<typename T>
    using safe = SafeInt<T>;

    using safe_int_error = SafeIntException;

    /** Check whether given exception is an overflow error.
     * @param error object
     * @return true iff given exception is an overflow error
     */
    inline bool is_overflow_error(const safe_int_error& error)
    {
        return error.m_code == SafeIntArithmeticOverflow;
    }

    /** Check whether given exception is an divide by zero error.
     * @param error object
     * @return true iff given exception is an divide by zero error
     */
    inline bool is_divide_by_zero_error(const safe_int_error& error)
    {
        return error.m_code == SafeIntDivideByZero;
    }
}

// Specializations of STL to safe types
namespace std
{
    // Hash
    template<typename T>
    struct hash<dice::safe<T>>
    {
        std::hash<T> hasher;

        std::size_t operator()(const dice::safe<T>& value) const
        {
            return hasher(static_cast<T>(value));
        }
    };

    // Max

    template<typename T>
    dice::safe<T> max(const dice::safe<T>& a, const dice::safe<T>& b)
    {
        return a.Max(b);
    }

    template<typename T>
    dice::safe<T> max(const dice::safe<T>& a, const T& b)
    {
        return a.Max(b);
    }

    template<typename T>
    dice::safe<T> max(const T& a, const dice::safe<T>& b)
    {
        return dice::safe<T>{ a }.Max(b);
    }

    // Min

    template<typename T>
    dice::safe<T> min(const dice::safe<T>& a, const dice::safe<T>& b)
    {
        return a.Min(b);
    }

    template<typename T>
    dice::safe<T> min(const dice::safe<T>& a, const T& b)
    {
        return a.Min(b);
    }

    template<typename T>
    dice::safe<T> min(const T& a, const dice::safe<T>& b)
    {
        return dice::safe<T>{ a }.Min(b);
    }

    // Swap

    template<typename T>
    void swap(dice::safe<T>& a, dice::safe<T>& b)
    {
        a.Swap(b);
    }

    // Print

    template<typename T>
    std::ostream& operator<<(std::ostream& output, dice::safe<T>& value)
    {
        output << static_cast<T>(value);
        return output;
    }

    template<typename T>
    std::string to_string(const dice::safe<T>& value)
    {
        return to_string(static_cast<T>(value));
    }
}

#endif // DICE_SAFE_HPP_