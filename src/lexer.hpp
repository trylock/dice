#ifndef DICE_LEXER_HPP_
#define DICE_LEXER_HPP_

#include <string>
#include <cctype>
#include <istream>

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
        explicit lexer(std::istream* input);
        lexer(const lexer&) = delete;
        void operator=(const lexer&) = delete;
        lexer(lexer&&) = default;
        lexer& operator=(lexer&&) = default;

        token read_token();

        inline const lexer_location& location() const 
        {
            return location_; 
        }
    private:
        std::istream* input_;
        lexer_location location_;

        void skip_space();
        int get_char();
    };
}

#endif // DICE_LEXER_HPP_