#ifndef DICE_SYMBOLS_HPP_
#define DICE_SYMBOLS_HPP_

#include <string>
#include <memory>
#include <type_traits>

#include "value.hpp"

namespace dice
{
    enum class symbol_type
    {
        plus, minus, times, divide, rel_op, roll_op, in,
        left_paren, right_paren, left_square_bracket, right_square_bracket,
        param_delim, semicolon, var, assign, number, func_id, id, end
    };

    struct symbol
    {
        // type of this symbol
        symbol_type type;

        /** Matched symbol string.
         * Lexeme is stored only for terminals where it is not a constant
         * (i.e. rel_op, number and id).
         */
        std::string lexeme = "";

        /** Value of the symbol.
         * It is only stored for numbers.
         */
        std::unique_ptr<base_value> value = nullptr;

        symbol() {}
        symbol(symbol_type type, std::string lexeme = "") 
            : type(type), lexeme(std::move(lexeme)) {}
        symbol(symbol_type type, std::unique_ptr<base_value> value) 
            : type(type), value(std::move(value)) {}

        inline bool operator==(const symbol& other) const
        {
            return type == other.type && lexeme == other.lexeme && value == other.value;
        }

        inline bool operator!=(const symbol& other) const
        {
            return !operator==(other);
        }
    };

    /** Convert given symbol to string
     * @param a symbol
     * @return string representation of the symbol 
     *         for debugging and error messages
     */
    std::string to_string(const symbol& symbol);

    /** Convert given symbol type to string
     * @param symbol type (terminal or non-terminal)
     * @return string representation of the symbol type 
     *         for debugging and error messages 
     */
    std::string to_string(symbol_type type);
}

#endif // DICE_SYMBOLS_HPP_