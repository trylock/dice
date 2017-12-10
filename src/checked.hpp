#ifndef DICE_CHECKED_HPP_
#define DICE_CHECKED_HPP_

#include <limits>
#include <stdexcept>

namespace dice
{
    // Check for arithmetic errors 
    template<typename T, bool is_signed = std::numeric_limits<T>::is_signed>
    class checked;

    // signed value specialization
    template<typename T>
    class checked<T, true>
    {
    public:
        checked() {}
        checked(const T& value) : value_(value) {}

        // allow copy
        checked(const checked&) = default;
        checked& operator=(const checked&) = default;

        // allow move
        checked(checked&&) = default;
        checked& operator=(checked&&) = default;

        /** Compute addition of the 2 values.
         * If an overflow occurs, the std::overflow_error is thrown.
         * If an uderflow occurs, the std::underflow_error is thrown.
         * @param other raw value
         * @return result of the addition as a checked type
         */
        auto operator+(const T& other_value) const
        {
            using namespace std;

            if (other_value > 0 && value_ > max() - other_value)
            {
                throw std::overflow_error{ 
                    to_string(value_) + " + " + to_string(other_value) };
            }

            if (other_value < 0 && value_ < min() - other_value)
            {
                throw std::underflow_error{ 
                    to_string(value_) + " + " + to_string(other_value) };
            }

            return checked<T>{ value_ + other_value };
        }

        /** Compute subtraction of the 2 values.
         * If an overflow occurs, the std::overflow_error is thrown.
         * If an uderflow occurs, the std::underflow_error is thrown.
         * @param other raw value
         * @return result of the subtraction as a checked type
         */
        auto operator-(const T& other_value) const
        {
            using namespace std;

            if (other_value < 0 && value_ > max() + other_value)
            {
                throw std::overflow_error{ 
                    to_string(value_) + " - " + to_string(other_value) };
            }

            if (other_value > 0 && value_ < min() + other_value)
            {
                throw std::underflow_error{ 
                    to_string(value_) + " - " + to_string(other_value) };
            }

            return checked<T>{ value_ - other_value };
        }

        /** Compute multiplication of the 2 values.
         * If an overflow occurs, the std::overflow_error is thrown.
         * If an uderflow occurs, the std::underflow_error is thrown.
         * @param other raw value
         * @return result of the multiplication as a checked type
         */
        auto operator*(const T& other_value) const
        {
            using namespace std;

            auto diff = max() + min();
            if (diff < 0)
            {
                if (value_ == -1 && other_value == min() ||
                    value_ == min() && other_value == -1)
                {
                    throw std::overflow_error{ 
                        to_string(value_) + " * " + to_string(other_value) 
                    };
                }
            }
            else if (diff > 0)
            {
                if (value_ == -1 && other_value == max() ||
                    value_ == max() && other_value == -1)
                {
                    throw std::underflow_error{
                        to_string(value_) + " * " + to_string(other_value) 
                    };
                }
            }

            if (other_value != 0 && value_ != 0 && other_value != -1)
            {
                // check whether there would be an overflow error
                if (other_value > 0 && value_ > max() / other_value ||
                    other_value < 0 && value_ < max() / other_value)
                {
                    throw std::overflow_error{ 
                        to_string(value_) + " * " + to_string(other_value) };
                }

                // check whether there would be an underflow error
                if (other_value > 0 && value_ < min() / other_value ||
                    other_value < 0 && value_ > min() / other_value)
                {
                    throw std::underflow_error{ 
                        to_string(value_) + " * " + to_string(other_value) };
                }
            }

            return checked<T>{ value_ * other_value };
        }

        /** Compute division of the 2 values.
         * If an overflow occurs, the std::overflow_error is thrown.
         * If the other_value is zero, the std::overflow_error is thrown.
         * @param other raw value
         * @return result of the division as a checked type
         */
        auto operator/(const T& other_value) const
        {
            using namespace std;

            if (other_value == 0)
            {
                throw std::overflow_error{ "Division by zero: " + 
                    to_string(value_) + " / 0" };
            }

            auto diff = max() + min();
            if (diff < 0)
            {
                if (value_ == min() && other_value == -1)
                {
                    throw std::overflow_error{ 
                        to_string(value_) + " / " + to_string(other_value) 
                    };
                }
            }
            else if (diff > 0)
            {
                if (value_ == max() && other_value == -1)
                {
                    throw std::underflow_error{
                        to_string(value_) + " / " + to_string(other_value) 
                    };
                }
            }

            return checked<T>{ value_ / other_value };
        }

        /** Compute unary minus of the value.
         * If an overflow occurs, the std::overflow_error is thrown.
         * @return result of the unary minus as a checked type
         */
        auto operator-() const
        {
            using namespace std;

            auto diff = max() + min();
            if (diff < 0 && value_ == min())
            {
                throw std::overflow_error{ "-" + to_string(value_) };
            }
            else if (diff > 0 && value_ == max())
            {
                throw std::underflow_error{ "-" + to_string(value_) };
            }
            return checked<T>{ -value_ };
        }

        // cast to type T
        operator T() const 
        { 
            return value_; 
        }

        // explicit value getter/setter
        T& value() 
        { 
            return value_; 
        }

        const T& value() const
        {
            return value_;
        }
    private:
        T value_;

        static constexpr T max()
        {
            return std::numeric_limits<T>::max();
        }

        static constexpr T min()
        {
            return std::numeric_limits<T>::min();
        }
    };

    // unsigned value specialization
    template<typename T>
    class checked<T, false>
    {
    public:
        checked() {}
        checked(const T& value) : value_(value) {}

        // allow copy
        checked(const checked&) = default;
        checked& operator=(const checked&) = default;

        // allow move
        checked(checked&&) = default;
        checked& operator=(checked&&) = default;

        /** Compute addition of the 2 values.
         * If an overflow occurs, the std::overflow_error is thrown.
         * @param other raw value
         * @return result of the addition as a checked type
         */
        auto operator+(const T& other_value) const
        {
            using namespace std;

            if (value_ > max() - other_value)
            {
                throw std::overflow_error{ 
                    to_string(value_) + " + " + to_string(other_value) };
            }

            return checked<T>{ value_ + other_value };
        }

        /** Compute subtraction of the 2 values.
         * If an uderflow occurs, the std::underflow_error is thrown.
         * @param other raw value
         * @return result of the subtraction as a checked type
         */
        auto operator-(const T& other_value) const
        {
            using namespace std;

            if (value_ <  other_value)
            {
                throw std::underflow_error{ 
                    to_string(value_) + " - " + to_string(other_value) };
            }

            return checked<T>{ value_ - other_value };
        }

        /** Compute multiplication of the 2 values.
         * If an overflow occurs, the std::overflow_error is thrown.
         * @param other raw value
         * @return result of the multiplication as a checked type
         */
        auto operator*(const T& other_value) const
        {
            using namespace std;

            // check whether there would be an overflow error
            if (other_value != 0 && value_ > max() / other_value)
            {
                throw std::overflow_error{ 
                    to_string(value_) + " * " + to_string(other_value) };
            }

            return checked<T>{ value_ * other_value };
        }

        /** Compute division of the 2 values.
         * If the other_value is zero, the std::overflow_error is thrown.
         * @param other raw value
         * @return result of the division as a checked type
         */
        auto operator/(const T& other_value) const
        {
            using namespace std;

            if (other_value == 0)
            {
                throw std::overflow_error{ "Division by zero: " + 
                    to_string(value_) + " / 0" };
            }

            return checked<T>{ value_ / other_value };
        }

        /** Compute unary minus of the value.
         * If an overflow occurs, the std::overflow_error is thrown.
         * @return result of the unary minus as a checked type
         */
        auto operator-() const
        {
            using namespace std;

            if (value_ != 0)
            {
                throw std::underflow_error{ "-" + to_string(value_) };
            }
            return checked<T>{ 0 };
        }

        // cast to type T
        operator T() const 
        { 
            return value_; 
        }

        // explicit value getter/setter
        T& value() 
        { 
            return value_; 
        }

        const T& value() const
        {
            return value_;
        }
    private:
        T value_;

        static constexpr T max()
        {
            return std::numeric_limits<T>::max();
        }
    };
}

#endif // DICE_CHECKED_HPP_