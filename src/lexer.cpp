#include "lexer.hpp"

dice::lexer::lexer(std::istream* input, errors* errs) : 
    input_(input), 
    errors_(errs) {}

dice::token dice::lexer::read_token()
{
    for (;;)
    {
        skip_space();

        auto current = static_cast<char>(get_char());
        if (input_->fail())
            return token{ token_type::end };

        auto next = static_cast<char>(input_->peek());
        if (current == '+')
            return token{ token_type::add };
        if (current == '-')
            return token{ token_type::sub };
        if (current == '*')
            return token{ token_type::mult };
        if (current == '/')
            return token{ token_type::div };
        if (std::tolower(current) == 'd' && !std::isalpha(next))
            return token{ token_type::roll_op };
        if (current == '(')
            return token{ token_type::left_parent };
        if (current == ')')
            return token{ token_type::right_parent };
        if (current == '[')
            return token{ token_type::left_square_bracket };
        if (current == ']')
            return token{ token_type::right_square_bracket };
        if (current == ',')
            return token{ token_type::param_delim };

        if (current == '<' || current == '>' || current == '!' || current == '=')
        {
            if (next == '=')
            {
                get_char();
                return token{ token_type::rel_op, std::string(1, current) + "=" };
            }
            else if (current == '<' || current == '>')
            {
                return token{ token_type::rel_op, std::string(1, current) };
            }
        }

        if (std::isdigit(current))
        {
            std::string value(1, current);
            for (;;)
            {
                if (std::isdigit(input_->peek()))
                    value += static_cast<char>(get_char());
                else
                    break;
            }
            return token{ token_type::number, value };
        }

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
                return token{ token_type::in };

            // otherwise, it's an identifier
            return token{ token_type::id, value };
        }

        // format an error message
        const char digits[] = { 
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F'
        };
        error("Unexpected character: '" + std::string(1, current) + "' (0x" + 
            std::string(1, digits[(current & 0xFF) >> 4]) + 
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
    errors_->add(location_.line, location_.col, message);
}


// Debug functions

std::string dice::to_string(token_type type)
{
    if (type == token_type::add)
        return "+";
    if (type == token_type::sub)
        return "-";
    if (type == token_type::mult)
        return "*";
    if (type == token_type::div)
        return "/";
    if (type == token_type::left_parent)
        return "(";
    if (type == token_type::right_parent)
        return ")";
    if (type == token_type::left_square_bracket)
        return "[";
    if (type == token_type::right_square_bracket)
        return "]";
    if (type == token_type::in)
        return "in";
    if (type == token_type::number)
        return "<number>";
    if (type == token_type::id)
        return "<identifier>";
    if (type == token_type::rel_op)
        return "<relational operator>";
    if (type == token_type::roll_op)
        return "<dice roll operator>";
    if (type == token_type::param_delim)
        return ",";
    if (type == token_type::end)
        return "<end of expression>";
    return "<unknown token type>";
}

std::string dice::to_string(const token& token)
{
    if (token.value.size() > 0)
        return to_string(token.type) + " '" + token.value + "'";
    return to_string(token.type);
}
