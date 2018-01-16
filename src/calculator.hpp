#ifndef DICE_CALCULATOR_HPP_
#define DICE_CALCULATOR_HPP_

#include <vector>
#include <string>
#include <istream>

#include "logger.hpp"
#include "environment.hpp"
#include "direct_interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"

namespace dice
{
    // Facade for script evaluation.
    struct calculator
    {
        using value_list = std::vector<std::unique_ptr<base_value>>;

        dice::logger log;
        dice::environment env;
        dice::direct_interpreter<dice::environment> interpret;

        calculator() : interpret(&env) {}

        /** Evaluate command in given input stream.
        * @param input character stream
        * @return evaluated values
        */
        value_list evaluate(std::istream* input);

        /** Process command in a string.
        * @param command as a string
        * @return evaluated values
        */
        value_list evaluate(const std::string& command);

        /** Enable interactive mode.
         * Interactive mode is optimized for repeated use of the environment.
         */
        void enable_interactive_mode();
    };
}

#endif // DICE_CALCULATOR_HPP_