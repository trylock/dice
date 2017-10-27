#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iomanip>

#include "logger.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "environment.hpp"
#include "interpreter.hpp"

// value formatting functions

void view(int value)
{
    std::cout << value << std::endl;
}

void view(double value)
{
    std::cout << value << std::endl;
}

template<typename ValueType, typename ProbabilityType>
void view(
    const dice::random_variable_decomposition<ValueType, ProbabilityType>& var)
{
    std::cout 
        << std::setw(13) << "Value" 
        << std::setw(20) << "Probability" << std::endl;
    for (auto&& pair : var.probability())
    {
        std::cout 
            << std::setw(13) << pair.first 
            << std::setw(20) << pair.second << std::endl;
    }
}

void print_value(const dice::base_value* value)
{
    if (value == nullptr)
        return;

    auto int_value = dynamic_cast<const dice::type_int*>(value);
    if (int_value != nullptr)
    {
        view(int_value->data());
        return;
    }
    
    auto double_value = dynamic_cast<const dice::type_double*>(value);
    if (double_value != nullptr)
    {
        view(double_value->data());
        return;
    }

    auto var_value = dynamic_cast<const dice::type_rand_var*>(value);
    if (var_value != nullptr)
    {
        view(var_value->data());
        return;
    }

    throw std::runtime_error("Unknown value.");
}

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        // concatenate arguments to form an expression
        std::string expr = argv[1];
        for (int i = 2; i < argc; ++i)
            expr += " " + std::string(argv[i]);
        std::stringstream input{ expr };

        // parse and interpret the expression
        dice::logger log;
        dice::lexer lexer{ &input, &log };
        dice::environment env;
        dice::interpreter<dice::environment> interpret{ &env };
        auto parser = dice::make_parser(&lexer, &log, &interpret);

        auto result = parser.parse(); 
        for (auto&& value : result)
        {
            print_value(value.get());
        }
    }
    else 
    {
        std::cerr << "Usage: dice_cli <expr>" << std::endl
            << "   <expr> is an arbitrary expression" << std::endl
            << "          multiple arguments will be concatenated" << std::endl;
        return 1;
    }
    return 0;
}