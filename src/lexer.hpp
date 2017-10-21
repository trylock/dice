#ifndef DICE_LEXER_HPP_
#define DICE_LEXER_HPP_

#include <string>
#include <sstream>
#include <cctype>
#include <istream>

#include "errors.hpp"

namespace dice 
{
    enum class token_type
    {
        number,
        id,
        rel_op,
        roll_op,
        add,
        sub,
        mult,
        div,
        left_parent,
        right_parent,
        left_square_bracket,
        right_square_bracket,
        param_delim,
        in,
        end
    };

    struct token
    {
        token_type type;
        std::string value;

        token() {}
        explicit token(token_type type) : type(type) {}
        token(token_type type, std::string value) :
            type(type),
            value(std::move(value)) {}
    };

    // convert token type to a human readable string
    std::string to_string(token_type type);

    // convert token value and type to a human readable string
    std::string to_string(const token& token);

    // location in the input stream
    struct lexer_location
    {
        int line;
        int col;

        lexer_location() : line(0), col(0) {}
        lexer_location(int line, int col = 0) : line(line), col(col) {}
    };

    class lexer 
    {
    public:
        lexer(std::istream* input, errors* errs);

        // disallow copy so that 2 lexers don't read from the same input
        lexer(const lexer&) = delete;
        void operator=(const lexer&) = delete;

        // allow move
        lexer(lexer&&) = default;
        lexer& operator=(lexer&&) = default;

        /** Read token at current location.
         * Decode a sequance of characters as a token and 
         * advance in the input stream.
         * @return token at current position
         */
        token read_token();

        // Return current location of the lexer in the input stream
        inline const lexer_location& location() const 
        {
            return location_; 
        }

    private:
        std::istream* input_;
        errors* errors_;
        lexer_location location_;

        // skip sequence of whitespace at current location
        void skip_space();

        // get character at current location
        int get_char();

        // report a lexer error
        void error(const std::string& message);
    };
}

#endif // DICE_LEXER_HPP_