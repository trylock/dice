#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <cassert>
#include <chrono>

#include "logger.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "environment.hpp"
#include "direct_interpreter.hpp"

// convert double to percent string
std::string format_probability(double prob)
{
    if (prob < 0.0001 && prob != 0)
    {
        return "< 0.01 %";
    }
    return std::to_string(prob * 100) + " %"; 
}

// value formatting functions

class formatting_visitor : public dice::value_visitor
{
public:
    void visit(dice::type_int* value) override
    {
        std::cout << value->data() << std::endl;
    }
    
    void visit(dice::type_double* value) override
    {
        std::cout << value->data() << std::endl;
    }

    void visit(dice::type_rand_var* value) override
    {
        const int width_value = 10;
        const int width_prob = 15;
        const int width_cdf = 15;
    
        // print table header
        std::cout << std::endl << termcolor::bold
            << std::setw(width_value) << "Value" 
            << std::setw(width_prob) << "PMF"  
            << std::setw(width_cdf) << "CDF"
            << std::endl << termcolor::reset;
    
        // sort PMF by value
        auto var = value->data().to_random_variable();
        dice::random_variable<int, double>::probability_list values{
            var.begin(),
            var.end()
        };
        std::sort(values.begin(), values.end(), [](auto&& a, auto&& b)
        {
            return a.first < b.first;
        });
    
        // print the random variable
        if (values.empty())
            return;
        
        double sum = 0;
        for (auto it = values.begin(); it != values.end(); ++it)
        {
            sum += it->second;
            std::cout 
                << std::setw(width_value) << it->first 
                << std::setw(width_prob) << format_probability(it->second)
                << std::setw(width_cdf) << format_probability(sum)
                << std::endl;
        }
    }
};

struct options
{
    // Command line arguments
    std::vector<std::string> args;
    // Input stream
    std::istream* input;
    // Should we print the executed script?
    bool verbose;
    // File path if input is a file, "<arguments>" otherwise
    std::string input_name;

    options(int argc, char** argv) : 
        args(argv, argv + argc), 
        input(nullptr),
        verbose(false),
        input_name("<arguments>")
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
                input_name = *it;
                input_file_.open(input_name);
                input = &input_file_;
                load_from_file = true;
            }
            else if (*it == "-v")
            {
                verbose = true;
            }
            else 
            {
                break;
            }
        }

        // load from arguments - concatenate the rest of the args
        if (!load_from_file && it != args.end())
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
    options opt{ argc, argv };

    if (opt.input != nullptr)
    {
        if (opt.input->fail())
        {
            std::cerr << "File not found: " << opt.input_name << std::endl;
            return 1;
        }

        // parse and interpret the expression
        dice::logger log;
        dice::environment env;
        dice::lexer<dice::logger> lexer{ opt.input, &log };
        dice::direct_interpreter<dice::environment> interpret{ &env };
        auto parser = dice::make_parser(&lexer, &log, &interpret);

        auto start = std::chrono::high_resolution_clock::now();
        auto result = parser.parse(); 
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start).count();
        
        formatting_visitor format;
        for (auto&& value : result)
        {
            if (value == nullptr)
                continue;
            value->accept(&format);
        }

        if (opt.verbose)
        {
            std::cout << "Evaluated in " << duration << " ms" << std::endl;
        }

        if (!log.empty())
            return 1;
    }
    else 
    {
        std::cerr << "Dice expressions interpreter." << std::endl 
            << std::endl
            << "Usage:" << std::endl 
            << "   ./dice_cli [options] [expression]" << std::endl
            << std::endl
            << "   [options]:" << std::endl
            << "      -f <file> load expression from file" << std::endl
            << "      -v verbose output (show executed script)" << std::endl
            << std::endl
            << "   [expression]:" << std::endl
            << "      A dice expression. Can be in multiple arguments." << std::endl
            << "      The program will join them wih a space." << std::endl;
        return 1;
    }
    return 0;
}