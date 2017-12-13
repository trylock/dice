#ifndef DICE_SAFE_HPP_
#define DICE_SAFE_HPP_

#include <limits>

#include <SafeInt3.hpp>

namespace dice
{
    template<typename T>
    using safe = SafeInt<T>;

    using safe_int_error = SafeIntException;

    /** Check whether given exception is an overflow error.
     * @param safe_int_error
     * @return true iff given exception is an overflow error
     */
    inline bool is_overflow_error(const safe_int_error& err)
    {
        return err.m_code == SafeIntArithmeticOverflow;
    }

    /** Check whether given exception is an divide by zero error.
     * @param safe_int_error
     * @return true iff given exception is an divide by zero error
     */
    inline bool is_divide_by_zero_error(const safe_int_error& err)
    {
        return err.m_code == SafeIntDivideByZero;
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

    // Numeric limits for safe types
    template<typename T>
    class numeric_limits<dice::safe<T>>
    {
    public:
        static constexpr auto is_specialized = numeric_limits<T>::is_specialized;
        static constexpr auto is_signed = numeric_limits<T>::is_signed;
        static constexpr auto is_integer = numeric_limits<T>::is_integer;
        static constexpr auto is_exact = numeric_limits<T>::is_exact;
        static constexpr auto has_infinity = numeric_limits<T>::has_infinity;
        static constexpr auto has_quiet_NaN = numeric_limits<T>::has_quiet_NaN;
        static constexpr auto has_signaling_NaN = numeric_limits<T>::has_signaling_NaN;
        static constexpr auto has_denorm = numeric_limits<T>::has_denorm;
        static constexpr auto has_denorm_loss = numeric_limits<T>::has_denorm_loss;
        static constexpr auto round_style = numeric_limits<T>::round_style;
        static constexpr auto is_iec559 = numeric_limits<T>::is_iec559;
        static constexpr auto is_bounded = numeric_limits<T>::is_bounded;
        static constexpr auto is_modulo = numeric_limits<T>::is_modulo;
        static constexpr auto digits = numeric_limits<T>::digits;
        static constexpr auto digits10 = numeric_limits<T>::digits10;
        static constexpr auto max_digits10 = numeric_limits<T>::max_digits10;
        static constexpr auto radix = numeric_limits<T>::radix;
        static constexpr auto min_exponent = numeric_limits<T>::min_exponent;
        static constexpr auto min_exponent10 = numeric_limits<T>::min_exponent10;
        static constexpr auto max_exponent = numeric_limits<T>::max_exponent;
        static constexpr auto max_exponent10 = numeric_limits<T>::max_exponent10;
        static constexpr auto traps = numeric_limits<T>::traps;
        static constexpr auto tinyness_before = numeric_limits<T>::tinyness_before;

        static constexpr dice::safe<T> lowest()
        {
            return numeric_limits<T>::lowest();
        }
        
        static constexpr dice::safe<T> min()
        {
            return numeric_limits<T>::min();
        }
        
        static constexpr dice::safe<T> max()
        {
            return numeric_limits<T>::max();
        }
        
        static constexpr dice::safe<T> epsilon()
        {
            return numeric_limits<T>::epsilon();
        }
        
        static constexpr auto round_error()
        {
            return numeric_limits<T>::round_error();
        }
        
        static constexpr dice::safe<T> infinity()
        {
            return numeric_limits<T>::infinity();
        }
        
        static constexpr dice::safe<T> quiet_NaN()
        {
            return numeric_limits<T>::quiet_NaN();
        }
        
        static constexpr dice::safe<T> signaling_NaN()
        {
            return numeric_limits<T>::signaling_NaN();
        }
        
        static constexpr dice::safe<T> denorm_min()
        {
            return numeric_limits<T>::denorm_min();
        }
    };
}

#endif // DICE_SAFE_HPP_