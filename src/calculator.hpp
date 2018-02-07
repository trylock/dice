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
    /** @brief Facade for script evaluation.
     *
     * Example usage:
     * @code
     * calculator c;
     * calculator::value_list result = c.evaluate("1d6");
     * {
     *      std::ifstrem file{ "input.dice" };
     *      result = c.evaluate(&file);
     * }
     * @endcode
     */
    struct calculator
    {
        using value_list = std::vector<std::unique_ptr<base_value>>;

        dice::logger log;
        dice::environment env;
        dice::direct_interpreter<dice::environment> interpret;

        calculator() : interpret(&env) {}

        /** @brief Evaluate command in given input stream.
         *
        * @param input character stream pointer
        * 
        * @return evaluated values
        */
        value_list evaluate(std::istream* input);

        /** @brief Process command in a string.
         *
        * @param command as a string
        * 
        * @return evaluated values
        */
        value_list evaluate(const std::string& command);

        /** @brief Enable interactive mode.
         *
         * Interactive mode is optimized for repeated use of the environment.
         * It allows variable redefinition.
         */
        void enable_interactive_mode();
    };
}

#endif // DICE_CALCULATOR_HPP_