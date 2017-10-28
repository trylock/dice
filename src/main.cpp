#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <iomanip>

#include "logger.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "environment.hpp"
#include "interpreter.hpp"

// conver a double value to percent
std::string format_probability(double prob)
{
    if (prob < 0.0001 && prob != 0)
    {
        return "< 0.01 %";
    }
    return std::to_string(prob * 100) + " %"; 
}

// value formatting functions

void print(int value)
{
    std::cout << value << std::endl;
}

void print(double value)
{
    std::cout << value << std::endl;
}

template<typename ValueType, typename ProbabilityType>
void print(
    const dice::random_variable_decomposition<ValueType, ProbabilityType>& var)
{
    const int width_value = 10;
    const int width_prob = 15;
    const int width_cdf = 15;

    std::cout 
        << "\e[1m" << std::setw(width_value) << "Value" 
        << "\e[1m" << std::setw(width_prob) << "PMF"  
        << "\e[1m" << std::setw(width_cdf) << "CDF"  
        << "\e[0m" 
        << std::endl;

    ProbabilityType sum = 0;
    for (auto&& pair : var.probability())
    {
        std::cout 
            << std::setw(width_value) << pair.first 
            << std::setw(width_prob) << format_probability(pair.second)
            << std::setw(width_cdf) << format_probability(sum)
            << std::endl;
        sum += pair.second;
    }
}

void print_delim()
{
    std::cout << std::endl;
}

void print_value(const dice::base_value* value)
{
    if (value == nullptr)
        return;
    print_delim();

    auto int_value = dynamic_cast<const dice::type_int*>(value);
    if (int_value != nullptr)
    {
        print(int_value->data());
        return;
    }
    
    auto double_value = dynamic_cast<const dice::type_double*>(value);
    if (double_value != nullptr)
    {
        print(double_value->data());
        return;
    }

    auto var_value = dynamic_cast<const dice::type_rand_var*>(value);
    if (var_value != nullptr)
    {
        print(var_value->data());
        return;
    }

    throw std::runtime_error("Unknown value.");
}


struct options
{
    // Command line arguments
    std::vector<std::string> args;
    // Input stream
    std::istream* input;

    options(int argc, char** argv) : args(argv, argv + argc)
    {
        parse();
    }

private:
    std::ifstream input_file_;
    std::stringstream input_mem_;

    void parse()
    {
        auto it = args.begin() + 1; // first arg is the file path
        
        bool load_from_file = false;
        for (; it != args.end(); ++it)
        {
            if (*it == "-f") // input file
            {
                assert(it + 1 != args.end());
                ++it;
                input_file_.open(*it);
                input = &input_file_;
                load_from_file = true;
            }
            else 
            {
                break;
            }
        }

        // load from arguments - concatenate the rest of the args
        if (!load_from_file)
        {
            std::string expr = *it++;
            for (; it != args.end(); ++it)
                expr += " " + *it;
            input_mem_ = std::stringstream{ expr };
            input = &input_mem_;
        }
    }
};

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        options opt{ argc, argv };

        // parse and interpret the expression
        dice::logger log;
        dice::lexer lexer{ opt.input, &log };
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
        std::cerr << "Usage: ./dice_cli [options] <expr>" << std::endl
            << "   -f <file> load expression from file" << std::endl;
        return 1;
    }
    return 0;
}