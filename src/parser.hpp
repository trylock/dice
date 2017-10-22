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
    template<typename TokenReader, typename Logger>
    class parser
    {
    public:
        using value_type = std::unique_ptr<base_value>;

        parser(TokenReader* reader, Logger* log) : lexer_(reader), log_(log) {}

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
        value_type parse()
        {
            lookahead_ = lexer_->read_token();
        
            // When using the expr as a start production, don't add 
            // the whole FOLLOW(expr) to the synchronizing tokens.
            // Use just the end symbol
            while (!in_first_expr())
            {
                error("Invalid token at the beginning of an expression: " +
                    to_string(lookahead_));
                eat(lookahead_.type);
                if (lookahead_.type == token_type::end)
                {
                    break;
                }
            }
        
            if (in_first_expr())
            {
                auto result = expr();
                // make sure we've processed the whole expression
                eat(token_type::end); 
                return result;
            }
            return make<type_int>(0);
        }

    private:
        lexer* lexer_;
        logger* log_;
        token lookahead_;
        environment environment_;

        value_type expr()
        {
            auto left = add();
            if (lookahead_.type == token_type::in)
            {
                eat(token_type::in);
                eat(token_type::left_square_bracket);
        
                // parse lower bound of the interval
                if (!check_add())
                {
                    error("Invalid operand for the lower bound of operator in");
                    return left;
                }
                auto lower_bound = add();
        
                eat(token_type::param_delim);
        
                // parse upper bound of the interval
                if (!check_add())
                {
                    error("Invalid operand for the upper bound of operator in");
                    return left;
                }
                auto upper_bound = add();
        
                eat(token_type::right_square_bracket);
        
                return environment_.call("in", 
                    std::move(left), 
                    std::move(lower_bound), 
                    std::move(upper_bound));
            }
            else if (lookahead_.type == token_type::rel_op)
            {
                auto op = lookahead_;
                eat(token_type::rel_op);
                auto right = add();
        
                // calculate the value
                return environment_.call(op.value, std::move(left), std::move(right));
            }
            return left;
        }

        value_type add()
        {
            std::string op;
            auto result = mult();
            for (;;)
            {
                // decide on the next operation
                if (lookahead_.type == token_type::add)
                {
                    eat(token_type::add);
                    op = "+";
                }
                else if (lookahead_.type == token_type::sub)
                {
                    eat(token_type::sub);
                    op = "-";
                }
                else 
                {
                    break;
                }
        
                // compute the operator if there won't be any parse error
                if (check_mult())
                {
                    result = environment_.call(op, std::move(result), mult());
                }
                else // otherwise, ignore the operator
                {
                    error("Invalid operand for binary operator " + op);
                }
            }
            return result;
        }

        value_type mult()
        {
            std::string op;
            auto result = dice_roll();
            for (;;)
            {
                // decide on the next operation
                if (lookahead_.type == token_type::mult)
                {
                    eat(token_type::mult);
                    op = "*";
                }
                else if (lookahead_.type == token_type::div)
                {
                    eat(token_type::div);
                    op = "/";
                }
                else 
                {
                    break;
                }

                // compute the operation if there won't be any parse error
                if (check_dice_roll())
                {
                    result = environment_.call(op, std::move(result), dice_roll());
                }
                else // otherwise, ignore the operator
                {
                    error("Invalid operand for binary operator " + op);
                }
            }
            return result;
        }

        value_type dice_roll()
        {
            // determine the sign
            int count = 0;
            for (;;)
            {
                if (lookahead_.type == token_type::sub)
                {
                    eat(token_type::sub);
                    ++count;
                }
                else 
                {
                    break;
                }
            }
        
            // parse dice roll
            auto result = factor();
            for (;;)
            {
                if (lookahead_.type == token_type::roll_op)
                {
                    eat(token_type::roll_op);
        
                    // only use the factor production if there won't be any parse error
                    if (check_factor())
                    {
                        result = environment_.call("roll", std::move(result), factor());
                    }
                    else // otherwise, ignore the operator 
                    {
                        error("Invalid operand for binary operator D (dice roll)");
                    }
                }
                else 
                {
                    break;
                }
            }
        
            // add sign
            if (count % 2 != 0)
                result = environment_.call("unary-", std::move(result));
            return result; 
        }

        value_type factor()
        {
            if (lookahead_.type == token_type::left_parent)
            {
                eat(token_type::left_parent);
                auto result = expr();
                eat(token_type::right_parent);
                return result;
            }
            else if (lookahead_.type == token_type::number)
            {
                auto value = std::atoi(lookahead_.value.c_str());
                eat(lookahead_.type);
                return make<type_int>(std::move(value));
            }
            else if (lookahead_.type == token_type::id)
            {
                // parse a function call
                auto id = lookahead_;
                eat(token_type::id);
                eat(token_type::left_parent);
                auto args = param_list();
                eat(token_type::right_parent);
                
                return environment_.call_var(id.value, args.begin(), args.end());
            }
            
            error("Expected " + to_string(token_type::left_parent) + ", " + 
                to_string(token_type::number) + " or " + 
                to_string(token_type::id) +  ", got " + 
                to_string(lookahead_) + "."); 
            return make<type_int>(0);
        }

        std::vector<value_type> param_list()
        {
            std::vector<value_type> args;
            if (lookahead_.type == token_type::right_parent)
            {
                return args; // no arguments
            }
        
            std::size_t number = 0;
            for (;;)
            {
                if (check_expr())
                {
                    args.emplace_back(expr());
                }
                else 
                {
                    error("Invalid parameter " + std::to_string(number));
                }
        
                if (lookahead_.type != token_type::param_delim)
                {
                    break;
                }
                eat(token_type::param_delim);
                ++number;
            }
            return args;
        }

        // true <=> next token is in FIRST(expr)
        bool in_first_expr() const 
        {
            return in_first_add();
        }

        // true <=> next token is in FIRST(add)
        bool in_first_add() const
        {
            return in_first_mult();
        }

        // true <=> next token is in FIRST(mult)
        bool in_first_mult() const
        {
            return in_first_dice_roll(); 
        }

        // true <=> next token is in FIRST(dice_roll)
        bool in_first_dice_roll() const
        {
            return lookahead_.type == token_type::sub || 
                in_first_factor();
        }

        // true <=> next token is in FIRST(factor)
        bool in_first_factor() const
        {
            return lookahead_.type == token_type::left_parent ||
                lookahead_.type == token_type::number ||
                lookahead_.type == token_type::id;
        }

        // true <=> next token is in FIRST(param_list)
        bool in_first_param_list() const
        {
            return lookahead_.type == token_type::right_parent || 
                in_first_expr();
        }

        // true <=> next token is in FOLLOW(expr)
        bool in_follow_expr() const
        {
            return lookahead_.type == token_type::end ||
                lookahead_.type == token_type::right_parent ||
                in_follow_param_list();    
        }

        // true <=> next token is in FOLLOW(add)
        bool in_follow_add() const
        {
            return lookahead_.type == token_type::add ||
                lookahead_.type == token_type::sub ||
                lookahead_.type == token_type::in ||
                lookahead_.type == token_type::rel_op ||
                lookahead_.type == token_type::param_delim ||
                lookahead_.type == token_type::right_square_bracket ||
                in_follow_expr();
        }

        // true <=> next token is in FOLLOW(mult)
        bool in_follow_mult() const
        {
            return lookahead_.type == token_type::mult ||
                lookahead_.type == token_type::div ||
                in_follow_add();
        }

        // true <=> next token is in FOLLOW(dice_roll)
        bool in_follow_dice_roll() const
        {
            return lookahead_.type == token_type::roll_op ||
               in_follow_mult();
        }

        // true <=> next token is in FOLLOW(factor)
        bool in_follow_factor() const
        {
            return in_follow_dice_roll();
        }

        // true <=> next token is in FOLLOW(param_list)
        bool in_follow_param_list() const
        {
            return lookahead_.type == token_type::param_delim ||
                lookahead_.type == token_type::right_parent; // FOLLOW(opt_params)
        }

        bool check_expr()
        {
            while (!in_follow_expr() && !in_first_expr())
            {
                error("Invalid token at the beginning of an expression: " + 
                    to_string(lookahead_));
                eat(lookahead_.type);
            }
            return in_first_expr();
        }

        bool check_add()
        {
            while (!in_first_add() && !in_follow_add())
            {
                error("Invalid token at the beginning of an addition: " + 
                    to_string(lookahead_));
                eat(lookahead_.type);
            }
            return in_first_add();
        }

        bool check_mult()
        {
            while (!in_first_mult() && !in_follow_mult())
            {
                error("Invalid token at the beginning of a multiplication: " + 
                    to_string(lookahead_));
                eat(lookahead_.type);
            }
            return in_first_mult();
        }

        bool check_dice_roll()
        {
            while (!in_first_dice_roll() && !in_follow_dice_roll())
            {
                error("Invalid token at the beginning of a dice roll: " + 
                    to_string(lookahead_));
                eat(lookahead_.type);
            }
            return in_first_dice_roll();
        }

        bool check_factor()
        {
            while (!in_first_factor() && !in_follow_factor())
            {
                error("Invalid token at the beginning of a factor: " + 
                    to_string(lookahead_));
                eat(lookahead_.type);
            }
            return in_first_factor();
        }

        bool check_param_list()
        {
            while (!in_first_param_list() && !in_follow_param_list())
            {
                error("Invalid token at the beginning of parameter list: " + 
                    to_string(lookahead_));
                eat(lookahead_.type);
            }
            return in_first_param_list();
        }

        /** Read token of given type from the input.
         * If there is no such token, "add" it at current position.
         * New token is read into the lookahead_ variable.
         * @param expected type
         */
        void eat(token_type type)
        {
            if (lookahead_.type != type)
            {
                // report the error and "insert" the token (i.e. assume it's there) 
                error("Expected " + 
                    to_string(type) + ", got " + 
                    to_string(lookahead_) + ".");
            }
            lookahead_ = lexer_->read_token();
        }

        /** Report a parsing error.
         * @param error message
         */
        void error(const std::string& message)
        {
            log_->error(
                lexer_->location().line, 
                lexer_->location().col, 
                message);
        }
    };
}

#endif // DICE_PARSER_HPP_