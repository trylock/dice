#include "symbols.hpp"

std::string dice::to_string(const symbol& value)
{
    auto str = to_string(value.type);
    if (!value.lexeme.empty())
        str += " '" + value.lexeme + "'";
    return str;
}

static const std::string symbol_names[] = {
    "+",
    "-",
    "*",
    "/",
    "<relational operator>",
    "<dice roll operator>",
    "in",
    "(",
    ")",
    "[",
    "]",
    ",",
    ";",
    "var",
    "=",
    "<number>",
    "<identifier>",
    "<end of input>"
};

std::string dice::to_string(symbol_type type)
{
    auto index = static_cast<std::size_t>(type);
    if (index >= std::extent<decltype(symbol_names)>::value)
        throw std::runtime_error("Invalid symbol type.");
    return symbol_names[index];
}