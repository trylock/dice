#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <cassert>

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
        std::cout << std::endl
            << std::setw(width_value) << "Value" 
            << std::setw(width_prob) << "PMF"  
            << std::setw(width_cdf) << "CDF"
            << std::endl;
    
        // sort PMF by value
        auto probability = value->data().probability();
        dice::random_variable<int, double>::probability_list values{
            probability.begin(),
            probability.end()
        };
        std::sort(values.begin(), values.end(), [](auto&& a, auto&& b)
        {
            return a.first < b.first;
        });
    
        // print the random variable
        if (values.empty())
            return;
        
        auto sum = values.front().second;
        for (auto it = values.begin() + 1; it != values.end(); ++it)
        {
            std::cout 
                << std::setw(width_value) << it->first 
                << std::setw(width_prob) << format_probability(it->second)
                << std::setw(width_cdf) << format_probability(sum)
                << std::endl;
            sum += it->second;
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

    options(int argc, char** argv) : 
        args(argv, argv + argc), 
        input(nullptr),
        verbose(false)
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
        // parse and interpret the expression
        dice::logger log;
        dice::lexer lexer{ opt.input, &log };
        dice::environment env;
        dice::interpreter<dice::environment> interpret{ &env };
        auto parser = dice::make_parser(&lexer, &log, &interpret);
        auto result = parser.parse(); 

        formatting_visitor format;
        for (auto&& value : result)
        {
            if (value == nullptr)
                continue;
            value->accept(&format);
        }
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