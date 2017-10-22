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
        bool in_first_expr() const;
        // true <=> next token is in FIRST(add)
        bool in_first_add() const;
        // true <=> next token is in FIRST(mult)
        bool in_first_mult() const;
        // true <=> next token is in FIRST(dice_roll)
        bool in_first_dice_roll() const;
        // true <=> next token is in FIRST(factor)
        bool in_first_factor() const;
        // true <=> next token is in FIRST(param_list)
        bool in_first_param_list() const;

        // true <=> next token is in FOLLOW(expr)
        bool in_follow_expr() const;
        // true <=> next token is in FOLLOW(add)
        bool in_follow_add() const;
        // true <=> next token is in FOLLOW(mult)
        bool in_follow_mult() const;
        // true <=> next token is in FOLLOW(dice_roll)
        bool in_follow_dice_roll() const;
        // true <=> next token is in FOLLOW(factor)
        bool in_follow_factor() const;
        // true <=> next token is in FOLLOW(param_list)
        bool in_follow_param_list() const;

        bool check_expr();
        bool check_add();
        bool check_mult();
        bool check_dice_roll();
        bool check_factor();
        bool check_param_list();

        void eat(token_type type);
        void error(const std::string& message);
    };
}

#endif // DICE_PARSER_HPP_