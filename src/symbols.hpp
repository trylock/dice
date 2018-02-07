#ifndef DICE_SYMBOLS_HPP_
#define DICE_SYMBOLS_HPP_

#include <string>
#include <memory>
#include <type_traits>

#include "value.hpp"

namespace dice
{
    /** @brief Type of a terminal symbol. */
    enum class symbol_type
    {
        plus, minus, times, divide, rel_op, roll_op, in,
        left_paren, right_paren, left_square_bracket, right_square_bracket,
        param_delim, semicolon, var, assign, number, func_id, id, end
    };

    /** @brief Terminal symbol returned by the lexer type. */
    struct symbol
    {
        /** @brief Type of this symbol */
        symbol_type type;

        /** @brief Matched symbol string.
         *
         * Lexeme is stored only for symbol_type::id, symbol_type::func_id and 
         * symbol_type::roll_op.
         */
        std::string lexeme;

        /** @brief Value of the symbol.
         *
         * It is only stored for numbers (symbol_type::number).
         */
        std::unique_ptr<base_value> value = nullptr;

        symbol() = default;

        /** @brief Create a symbol with just a type.
         *
         * @param type of the symbol
         */
        symbol(symbol_type type) : type(type) {}

        /** @brief Create a symbol with lexeme value filled.
         *
         * @param type of the symbol
         * @param lexeme of the symbol
         */
        symbol(symbol_type type, std::string lexeme) 
            : type(type), lexeme(std::move(lexeme)) {}

        /** @brief Create a symbol with value filled
         *
         * @param type of the symbol
         * @param value of the symbol
         */
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

    /** @brief Convert given symbol to string
     *
     * @param symbol 
     * 
     * @return string representation of the symbol for debugging and error 
     *         messages.
     */
    std::string to_string(const symbol& symbol);

    /** @brief Convert given symbol type to string
     *
     * @param type of a symbol
     * 
     * @return string representation of the symbol type for debugging and 
     *         error messages.
     */
    std::string to_string(symbol_type type);
}

#endif // DICE_SYMBOLS_HPP_