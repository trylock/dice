#include "lexer.hpp"

dice::lexer::lexer(std::istream* input, logger* log) : 
    input_(input), 
    log_(log) {}

dice::symbol dice::lexer::read_token()
{
    for (;;)
    {
        skip_space();

        auto current = static_cast<char>(get_char());
        if (input_->fail())
            return { symbol_type::end };

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
                if (!std::isalnum(input_->peek()) && input_->peek() != '_')
                    break;
                value += static_cast<char>(get_char());
            }

            // check if it's a reserved keyword
            if (value == "in")
                return symbol{ symbol_type::in };

            // otherwise, it's an identifier
            return symbol{ symbol_type::id, value };
        }

        // format an error message
        const char digits[] = { 
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F'
        };
        error("Unexpected character: '" + std::string(1, current) + "' (0x" + 
            std::string(1, digits[(current >> 4) & 0xF]) + 
            std::string(1, digits[current & 0xF])  + ").");
    }
}

void dice::lexer::skip_space()
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

int dice::lexer::get_char() 
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

void dice::lexer::error(const std::string& message)
{
    log_->error(location_.line, location_.col, message);
}
