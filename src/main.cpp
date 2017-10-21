#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iomanip>

#include "parser.hpp"

// Formatting functions
std::string to_percet_str(double value)
{
    if (value > 0 && value < 0.0001)
        return "< 0.01 %";
    return std::to_string(value * 100) + " %";
}

std::ostream& bold(std::ostream& output)
{
    return output << "\e[1m";
}

std::ostream& red(std::ostream& output)
{
    return output << "\e[1;31m";
}

std::ostream& blue(std::ostream& output)
{
    return output << "\e[1;34m";
}

std::ostream& reset(std::ostream& output)
{
    return output << "\e[0m";
}

// Output message functions
template<typename ErrorType>
void error_message(ErrorType& error, dice::lexer_location loc)
{
    std::cerr 
        << bold << "[line: " << loc.line
                << "][column: " << loc.col << "] " 
        << reset
        << red << "error: " << reset << error.what() << std::endl << std::endl;
}

void print_result(const dice::random_variable<int, double>& value)
{
    // print constant values as constants
    if (value.is_constant())
    {
        std::cout << value.value() << std::endl;
        return;
    }

    // print the distribution sorted by value
    std::vector<std::pair<int, double>> dist{ 
        value.probability().begin(), 
        value.probability().end() 
    };
    std::sort(dist.begin(), dist.end(), [](auto&& a, auto&& b) 
    {
        return a.first < b.first;
    });

    std::cout << bold << std::setw(10) << std::right << "Value" << reset;
    std::cout << bold << std::setw(15) << std::right << "Probability" << reset << std::endl;

    for (auto&& pair : dist)
    {
        std::cout 
            << bold << std::setw(10) << std::right << pair.first << reset << ": " 
            << std::setw(12) << to_percet_str(pair.second) << std::endl;
    }
}

void print_result(std::unique_ptr<dice::base_value>&& value)
{
    auto scalar_int = dynamic_cast<dice::type_int*>(value.get());
    if (scalar_int != nullptr)
    {
        std::cout << scalar_int->data() << std::endl;
        return;
    }
    auto scalar_double = dynamic_cast<dice::type_double*>(value.get());
    if (scalar_double != nullptr)
    {
        std::cout << scalar_double->data() << std::endl;
        return;
    }
    auto rv = dynamic_cast<dice::type_rand_var*>(value.get());
    print_result(rv->data());
}

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        // concatenate arguments to form an expression
        std::string expr;
        for (int i = 1; i < argc; ++i)
            expr += argv[i];
        std::stringstream input{ expr };

        // parse and interpret the expression
        dice::lexer lexer{ &input, &std::cerr };
        dice::parser parser{ &lexer };
        dice::parser::value_type result;
        print_result(parser.parse());
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