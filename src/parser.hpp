#ifndef DICE_PARSER_HPP_
#define DICE_PARSER_HPP_

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "value.hpp"
#include "lexer.hpp"
#include "environment.hpp"
#include "random_variable.hpp"

namespace dice
{
    // an error called during parsing when it fails
    class parse_error : public std::runtime_error
    {
    public:
        parse_error(const std::string& message) : std::runtime_error(message) {}
    };

    // dice expression parser
    class parser
    {
    public:
        using value_type = std::unique_ptr<base_value>;

        explicit parser(lexer* l);

        /** Parse expression provided by the lexer.
         * Operators are left associative unless stated otherwise.
         * List of operators (from lowest to highest precedence): 
         * -#  <, <=, !=, ==, >=, >, in (relational operators, not assosiative)
         * -#  + (add), - (subtract)
         * -#  * (multiply), / (divide) 
         * -#  - (unary minus)
         * -#  D|d (roll dice)
         * @return calculated value 
         */
        value_type parse();

    private:
        lexer* lexer_;
        token lookahead_;
        environment environment_;

        value_type expr();
        value_type add();
        value_type mult();
        value_type dice_roll();
        value_type factor();
        std::vector<value_type> param_list();

        void eat(token_type type);
    };
}

#endif // DICE_PARSER_HPP_