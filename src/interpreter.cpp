#include "interpreter.hpp"

const std::array<dice::symbol_type, 8> dice::formatter::add_space_before{{
    symbol_type::plus,
    symbol_type::minus,
    symbol_type::times,
    symbol_type::divide,
    symbol_type::rel_op,
    symbol_type::roll_op,
    symbol_type::in,
    symbol_type::assign,
}};

const std::array<dice::symbol_type, 10> dice::formatter::add_space_after{{
    symbol_type::plus,
    symbol_type::minus,
    symbol_type::times,
    symbol_type::divide,
    symbol_type::rel_op,
    symbol_type::roll_op,
    symbol_type::in,
    symbol_type::assign,
    symbol_type::var,
    symbol_type::param_delim,
}};

const std::array<dice::symbol_type, 1> dice::formatter::add_newline{{
    symbol_type::semicolon
}};

const std::array<dice::symbol_type, 2> dice::formatter::keywords{{
    symbol_type::in,
    symbol_type::var,
}};

const char dice::formatter::highlight_keyword[] = "\e[35;1m";
const char dice::formatter::highlight_number[] = "\e[93m";
const char dice::formatter::highlight_id[] = "\e[97m";

void dice::formatter::add(const symbol& symbol)
{
    // add a space before the symbol 
    for (auto&& type : add_space_before)
    {
        if (type == symbol.type)
        {
            output_ << ' ';
            break;
        }
    }

    // enable highlighting
    if (symbol.type == symbol_type::number)
    {
        output_ << highlight_number;
    }
    else if (symbol.type == symbol_type::id)
    {
        output_ << highlight_id;
    }
    for (auto&& type : keywords)
    {
        if (type == symbol.type)
        {
            output_ << highlight_keyword;
            break;
        }
    }

    // print symbol value
    if (symbol.type == symbol_type::roll_op)
    {
        output_ << 'd';
    }
    else if (symbol.lexeme.empty())
    {
        if (symbol.type != symbol_type::semicolon)
            output_ << to_string(symbol.type);
    }
    else 
    {
        output_ << symbol.lexeme;
    }

    // disable highlighting
    output_ << "\e[0m";

    // add a space after the symbol 
    for (auto&& type : add_space_after)
    {
        if (type == symbol.type)
        {
            output_ << ' ';
            break;
        }
    }
    
    // add a newline after the symbol 
    for (auto&& type : add_newline)
    {
        if (type == symbol.type)
        {
            output_ << std::endl;
            break;
        }
    }
}