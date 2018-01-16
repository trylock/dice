#include "calculator.hpp"

void dice::calculator::enable_interactive_mode()
{
    interpret.set_variable_redefinition(true);
}

dice::calculator::value_list dice::calculator::evaluate(std::istream* input)
{
    dice::lexer<dice::logger> lexer{ input, &log };
    auto parser = dice::make_parser(&lexer, &log, &interpret);
    return parser.parse();
}

dice::calculator::value_list dice::calculator::evaluate(const std::string& command)
{
    std::stringstream input{ command };
    return evaluate(&input);
}