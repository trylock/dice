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
         * Decode a sequance of characters as a token and advance in the input 
         * stream. Skip all whitespace characters after the token (that is
         * location after calling read_token points to the beginning of the 
         * next token)
         * @return token at current position
         */
        symbol read_token()
        {
            auto token = read_token_internal();
            skip_space();
            return token;
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

        auto read_token_internal()
        {
            for (;;)
            {
                skip_space();

                auto current = get_char();
                if (input_->fail())
                    return symbol{ symbol_type::end };

                auto next = input_->peek();

                // skip comments
                if (current == '/' && next == '/')
                {
                    for (;;)
                    {
                        auto value = get_char();
                        if (value == '\n' || input_->fail())
                        {
                            break;
                        }
                    }
                    continue; // start parsing next token
                }

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
                    return parse_number(current);
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
                unexpected_character(current);
            }
        }

        symbol parse_number(char current_char)
        {
            std::string value(1, current_char);
            std::size_t num_dots = 0;
            std::size_t invalid_pos = 0;
            for (;;)
            {
                if (std::isdigit(input_->peek()))
                {
                    value += static_cast<char>(get_char());
                }
                else if (input_->peek() == '.')
                {
                    if (num_dots == 1)
                    {
                        invalid_pos = value.size();
                    }
                    value += static_cast<char>(get_char());
                    ++num_dots;
                }
                else
                {
                    break;
                }
            }

            // report malformed values
            if (value.back() == '.' || invalid_pos > 0)
            {
                error("Malformed number: '" + value + "'");
            }

            // fix malformed values
            if (value.back() == '.')
            {
                value += "0";
            }
            else if (invalid_pos > 0)
            {
                value = value.substr(0, invalid_pos);
            }

            // parse the number
            auto dot_pos = value.find('.');
            if (dot_pos == std::string::npos) // integer number
            {
                std::uint64_t data = 0;
                auto is_overflow = false;
                for (auto&& c : value)
                {
                    data *= 10;
                    data += c - '0';
                    if (data > std::numeric_limits<int>::max())
                    {
                        is_overflow = true;
                    }
                }

                if (is_overflow)
                {
                    error("Value out of range: '" + value + "'");
                }

                auto mask = static_cast<std::uint64_t>(
                    std::numeric_limits<int>::max());
                return symbol{
                    symbol_type::number,
                    make<type_int>(static_cast<int>(data & mask))
                };
            }
            else // decimal number
            {
                try
                {
                    return symbol{
                        symbol_type::number,
                        make<type_double>(std::stod(value))
                    };
                }
                catch (std::out_of_range&)
                {
                    error("Value out of range: '" + value + "'");
                    return symbol{
                        symbol_type::number,
                        make<type_double>(0.0)
                    };
                }
            }
        }

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

        /** Report an unexpected character error.
         * @param value of the character
         */
        void unexpected_character(char value)
        {
            const char digits[] = {
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                'A', 'B', 'C', 'D', 'E', 'F'
            };
            error("Unexpected character: '" + std::string(1, value) +
                "' (0x" + std::string(1, digits[(value >> 4) & 0xF]) +
                std::string(1, digits[value & 0xF]) + ").");
        }
    };
}

#endif // DICE_LEXER_HPP_