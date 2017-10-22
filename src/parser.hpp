#ifndef DICE_PARSER_HPP_
#define DICE_PARSER_HPP_

#include <memory>
#include <string>
#include <vector>
#include <ostream>
#include <functional>
#include <unordered_map>

#include "logger.hpp"
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

        parser(lexer* l, logger* log);

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
        logger* log_;
        token lookahead_;
        environment environment_;

        value_type expr();
        value_type add();
        value_type mult();
        value_type dice_roll();
        value_type factor();
        std::vector<value_type> param_list();

        // true <=> next token is in FIRST(expr)
        bool is_valid_expr() const;
        // true <=> next token is in FIRST(add)
        bool is_valid_add() const;
        // true <=> next token is in FIRST(mult)
        bool is_valid_mult() const;
        // true <=> next token is in FIRST(dice_roll)
        bool is_valid_dice_roll() const;
        // true <=> next token is in FIRST(factor)
        bool is_valid_factor() const;

        void eat(token_type type);
        void error(const std::string& message);
    };
}

#endif // DICE_PARSER_HPP_