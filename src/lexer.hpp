#ifndef DICE_LEXER_HPP_
#define DICE_LEXER_HPP_

#include <string>
#include <sstream>
#include <cctype>
#include <istream>

#include "logger.hpp"
#include "symbols.hpp"

namespace dice 
{
    // location in the input stream
    struct lexer_location
    {
        int line = 0;
        int col = 0;

        lexer_location() {}
        lexer_location(int line, int col) : line(line), col(col) {}
    };

    template<typename Logger>
    class lexer 
    {
    public:
        lexer(std::istream* input, Logger* log) : 
            input_(input), 
            log_(log) {}

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
        symbol read_token()
        {
            for (;;)
            {
                skip_space();

                auto current = static_cast<char>(get_char());
                if (input_->fail())
                    return symbol{ symbol_type::end };

                auto next = static_cast<char>(input_->peek());
                if (current == '+')
                    return symbol{ symbol_type::plus };
                if (current == '-')
                    return symbol{ symbol_type::minus };
                if (current == '*')
                    return symbol{ symbol_type::times };
                if (current == '/')
                    return symbol{ symbol_type::divide };
                if (std::tolower(current) == 'd' && !std::isalpha(next))
                    return symbol{ symbol_type::roll_op };
                if (current == '(')
                    return symbol{ symbol_type::left_paren };
                if (current == ')')
                    return symbol{ symbol_type::right_paren };
                if (current == '[')
                    return symbol{ symbol_type::left_square_bracket };
                if (current == ']')
                    return symbol{ symbol_type::right_square_bracket };
                if (current == ',')
                    return symbol{ symbol_type::param_delim };
                if (current == ';')
                    return symbol{ symbol_type::semicolon };

                if (current == '<' || current == '>' || current == '!' || 
                    current == '=')
                {
                    if (next == '=')
                    {
                        get_char();
                        return symbol{ symbol_type::rel_op, 
                            std::string(1, current) + "=" };
                    }
                    else if (current == '<' || current == '>')
                    {
                        return symbol{ symbol_type::rel_op, 
                            std::string(1, current) };
                    }
                    else if (current == '=')
                    {
                        return symbol{ symbol_type::assign };
                    }
                }

                // parse numbers
                if (std::isdigit(current))
                {
                    std::string value(1, current);
                    for (;;)
                    {
                        if (std::isdigit(input_->peek()))
                        {
                            value += static_cast<char>(get_char());
                        }
                        else if (input_->peek() == '.')
                        {
                            value += static_cast<char>(get_char());
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (value.back() == '.')
                    {
                        error("Invalid floating point number. " 
                            "Decimal part must not be empty: " + value);
                        value += "0";
                    }

                    return symbol{
                        symbol_type::number,
                        value
                    };
                }

                // parse identifiers
                if (std::isalpha(current))
                {
                    // read a string value
                    std::string value(1, current);
                    for (;;)
                    {
                        if (input_->peek() == EOF)
                            break;
                        if (!std::isalnum(input_->peek()) && 
                            input_->peek() != '_')
                            break;
                        value += static_cast<char>(get_char());
                    }

                    // check if it's a reserved keyword
                    if (value == "in")
                        return symbol{ symbol_type::in };
                    else if (value == "var")
                        return symbol{ symbol_type::var };

                    // distinguish function and other identifiers
                    skip_space();
                    if (input_->peek() == '(')
                        return symbol{ symbol_type::func_id, value };

                    // otherwise, it's an identifier
                    return symbol{ symbol_type::id, value };
                }

                // format an error message
                const char digits[] = { 
                    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                    'A', 'B', 'C', 'D', 'E', 'F'
                };
                error("Unexpected character: '" + std::string(1, current) + 
                    "' (0x" +  std::string(1, digits[(current >> 4) & 0xF]) + 
                    std::string(1, digits[current & 0xF])  + ").");
            }
        }

        // Return current location of the lexer in the input stream
        inline const lexer_location& location() const 
        {
            return location_; 
        }
    private:
        std::istream* input_;
        Logger* log_;
        lexer_location location_;

        // skip sequence of whitespace at current location
        void skip_space()
        {
            for (;;)
            {
                if (input_->peek() == EOF)
                    break;
                if (!std::isspace(input_->peek()))
                    break;
                get_char();
            }
        }

        // get character at current location
        int get_char()
        {
            auto c = input_->get();
            if (c == '\n')
            {
                ++location_.line;
                location_.col = 0;
            }
            else 
            {
                ++location_.col;
            }
            return c;
        }

        // report a lexer error
        void error(const std::string& message)
        {
            log_->error(location_.line, location_.col, message);
        }

    };
}

#endif // DICE_LEXER_HPP_