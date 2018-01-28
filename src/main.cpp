#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <cassert>
#include <chrono>

#include <linenoise.h>
#include <termcolor.hpp>

#include "logger.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "environment.hpp"
#include "direct_interpreter.hpp"
#include "calculator.hpp"

/** Format probability as a human readable string.
 * @param probability
 * @return probability as a formated string
 */
std::string format_probability(double probability)
{
    if (probability < 0.0001 && probability != 0)
    {
        return "< 0.01 %";
    }
    return std::to_string(probability * 100) + " %";
}

// Format any value type and print it to standard output.
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
    // File path if input is a file, "<arguments>" otherwise
    std::string input_name;

    options(int argc, char** argv) : 
        args(argv, argv + argc), 
        input(nullptr),
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
                if (it + 1 == args.end())
                {
                    throw std::invalid_argument{ 
                        "Missing argument (file name) for the -f option." };
                }
                ++it;
                input_name = *it;
                input_file_.open(input_name);
                input = &input_file_;
                load_from_file = true;
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

struct command_reader
{
    command_reader()
    {
        linenoiseInstallWindowChangeHandler();
    }

    ~command_reader()
    {
        linenoiseHistoryFree();
    }

    /** Read line from the input.
     * The line is saved to history.
     * @param out_line read line
     * @return ture iff we should continue reading
     */
    bool try_read(std::string& out_line)
    {
        auto result = linenoise("> ");
        if (result == nullptr)
            return false;
        linenoiseHistoryAdd(result);
        out_line = result;
        free(result);
        return true;
    }
};

/** Print computed values to standard output.
 * @param values list (result of the dice::parser::parse() method)
 */
template<typename ValueList>
void print_values(const ValueList& values)
{
    formatting_visitor format;
    for (auto&& value : values)
    {
        if (value == nullptr)
            continue;
        value->accept(&format);
    }
}

int main(int argc, char** argv)
{
    try
    {
        options opt{ argc, argv };
        dice::calculator calc;

        if (opt.input != nullptr)
        {
            if (opt.input->fail())
            {
                std::cerr << "File not found: " << opt.input_name << std::endl;
                return 1;
            }

            print_values(calc.evaluate(opt.input));
        }
        else
        {
            std::cout
                << "Dice expression probability calculator (interactive mode)"
                << std::endl
                << std::endl
                << "Type 'exit' to exit the application." << std::endl
                << "Type an expression to evaluate it." << std::endl
                << std::endl;

            calc.enable_interactive_mode();
            command_reader reader;
            for (;;)
            {
                std::string line;
                if (!reader.try_read(line))
                {
                    break;
                }

                if (line == "exit" || line == "end")
                {
                    break;
                }

                print_values(calc.evaluate(line));
            }
        }
    }
    catch (std::invalid_argument& error)
    {
        std::cerr << error.what() << std::endl;
        return 1;
    }
    return 0;
}