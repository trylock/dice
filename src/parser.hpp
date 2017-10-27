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
#include "symbols.hpp"
#include "environment.hpp"
#include "random_variable.hpp"

namespace dice
{
    // dice expression parser
    template<typename Lexer, typename Logger, typename Environment>
    class parser
    {
    public:
        using value_type = std::unique_ptr<base_value>;

        parser(Lexer* reader, Logger* log, Environment* env) : 
            lexer_(reader), 
            log_(log),
            env_(env) {}

        /** Parse expression provided by the lexer.
         * Operators are left associative unless stated otherwise.
         * List of operators (from lowest to highest precedence): 
         * -#  = (assignment, non-associative)
         * -#  <, <=, !=, ==, >=, >, in (relational operators, non-assosiative)
         * -#  + (add), - (subtract)
         * -#  * (multiply), / (divide) 
         * -#  - (unary minus)
         * -#  D|d (roll dice)
         * @return vector of computed values
         */
        std::vector<value_type> parse()
        {
            lookahead_ = lexer_->read_token();
            auto result = stmts();
            // make sure we've processed the whole expression
            eat(symbol_type::end); 
            return result;
        }

    private:
        Lexer* lexer_;
        Logger* log_;
        Environment* env_;
        symbol lookahead_;

        std::vector<value_type> stmts()
        {
            while (lookahead_.type != symbol_type::end && !in_first_stmt())
            {
                error("Invalid token at the beginning of an expression: " +
                    to_string(lookahead_));
                eat(lookahead_.type);
            }

            std::vector<value_type> values;
            if (lookahead_.type == symbol_type::end)
            {
                return values;
            }
            
            values.push_back(stmt());
            for (;;)
            {
                if (lookahead_.type == symbol_type::semicolon)
                {
                    eat(lookahead_.type);
                    if (check_stmt())
                    {
                        values.push_back(stmt());
                    }
                    else 
                    {
                        error("Invalid statement.");
                    }
                }
                else 
                {
                    break;
                }
            }
            return values;
        }

        bool in_first_stmt() const
        {
            return lookahead_.type == symbol_type::var ||
                in_first_expr();
        }

        bool in_follow_stmt() const
        {
            return lookahead_.type == symbol_type::semicolon ||
                // FOLLOW(stmts):
                lookahead_.type == symbol_type::end;
        }

        bool check_stmt() 
        {
            while (!in_follow_stmt() && !in_first_stmt())
            {
                error("Invalid token at the beginning of a statement: " + 
                    to_string(lookahead_));
                eat(lookahead_.type);
            }
            return in_first_stmt();
        }

        value_type stmt()
        {
            if (lookahead_.type == symbol_type::var)
            {
                eat(lookahead_.type);
                auto id = lookahead_;
                eat(symbol_type::id);
                eat(symbol_type::assign);
                auto value = expr();

                env_->set_var(id.lexeme, std::move(value));
                return nullptr;
            }
            return expr();
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

        // true <=> next token is in FIRST(expr)
        bool in_first_expr() const 
        {
            return in_first_add();
        }

        // true <=> next token is in FOLLOW(expr)
        bool in_follow_expr() const
        {
            return lookahead_.type == symbol_type::end ||
                lookahead_.type == symbol_type::right_paren ||
                in_follow_param_list();    
        }

        value_type expr()
        {
            auto left = add();
            if (lookahead_.type == symbol_type::in)
            {
                eat(symbol_type::in);
                eat(symbol_type::left_square_bracket);
        
                // parse lower bound of the interval
                if (!check_add())
                {
                    error(
                        "Invalid operand for the lower bound of operator in");
                    return left;
                }
                auto lower_bound = add();
        
                eat(symbol_type::param_delim);
        
                // parse upper bound of the interval
                if (!check_add())
                {
                    error(
                        "Invalid operand for the upper bound of operator in");
                    return left;
                }
                auto upper_bound = add();
        
                eat(symbol_type::right_square_bracket);
        
                return env_->call("in", 
                    std::move(left), 
                    std::move(lower_bound), 
                    std::move(upper_bound));
            }
            else if (lookahead_.type == symbol_type::rel_op)
            {
                auto op = lookahead_;
                eat(symbol_type::rel_op);
                if (check_add())
                {
                    auto right = add();
                    return env_->call(op.lexeme, std::move(left), 
                        std::move(right));
                }
                else 
                {
                    error("Invalid operand for " + to_string(op));
                }
            }
            return left;
        }
        
        // true <=> next token is in FIRST(add)
        bool in_first_add() const
        {
            return in_first_mult();
        }
        
        // true <=> next token is in FOLLOW(add)
        bool in_follow_add() const
        {
            return lookahead_.type == symbol_type::plus ||
                lookahead_.type == symbol_type::minus ||
                lookahead_.type == symbol_type::in ||
                lookahead_.type == symbol_type::rel_op ||
                lookahead_.type == symbol_type::param_delim ||
                lookahead_.type == symbol_type::right_square_bracket ||
                in_follow_expr();
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

        value_type add()
        {
            std::string op;
            auto result = mult();
            for (;;)
            {
                // decide on the next operation
                if (lookahead_.type == symbol_type::plus)
                {
                    eat(symbol_type::plus);
                    op = "+";
                }
                else if (lookahead_.type == symbol_type::minus)
                {
                    eat(symbol_type::minus);
                    op = "-";
                }
                else 
                {
                    break;
                }
        
                // compute the operator if there won't be any parse error
                if (check_mult())
                {
                    result = env_->call(op, std::move(result), mult());
                }
                else // otherwise, ignore the operator
                {
                    error("Invalid operand for binary operator " + op);
                }
            }
            return result;
        }

        // true <=> next token is in FIRST(mult)
        bool in_first_mult() const
        {
            return in_first_dice_roll(); 
        }

        // true <=> next token is in FOLLOW(mult)
        bool in_follow_mult() const
        {
            return lookahead_.type == symbol_type::times ||
                lookahead_.type == symbol_type::divide ||
                in_follow_add();
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

        value_type mult()
        {
            std::string op;
            auto result = dice_roll();
            for (;;)
            {
                // decide on the next operation
                if (lookahead_.type == symbol_type::times)
                {
                    eat(symbol_type::times);
                    op = "*";
                }
                else if (lookahead_.type == symbol_type::divide)
                {
                    eat(symbol_type::divide);
                    op = "/";
                }
                else 
                {
                    break;
                }

                // compute the operation if there won't be any parse error
                if (check_dice_roll())
                {
                    result = env_->call(op, std::move(result), dice_roll());
                }
                else // otherwise, ignore the operator
                {
                    error("Invalid operand for binary operator " + op);
                }
            }
            return result;
        }

        // true <=> next token is in FIRST(dice_roll)
        bool in_first_dice_roll() const
        {
            return lookahead_.type == symbol_type::minus || 
                in_first_factor();
        }
        
        // true <=> next token is in FOLLOW(dice_roll)
        bool in_follow_dice_roll() const
        {
            return lookahead_.type == symbol_type::roll_op ||
               in_follow_mult();
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

        value_type dice_roll()
        {
            // determine the sign
            int count = 0;
            for (;;)
            {
                if (lookahead_.type == symbol_type::minus)
                {
                    eat(symbol_type::minus);
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
                if (lookahead_.type == symbol_type::roll_op)
                {
                    eat(symbol_type::roll_op);

                    // only use the factor production if there won't be any 
                    // parse error
                    if (check_factor())
                    {
                        result = env_->call("__roll_op", std::move(result), 
                            factor());
                    }
                    else // otherwise, ignore the operator 
                    {
                        error("Invalid operand for binary operator "  
                            "D (dice roll)");
                    }
                }
                else 
                {
                    break;
                }
            }
        
            // add sign
            if (count % 2 != 0)
                result = env_->call("unary-", std::move(result));
            return result; 
        }

        // true <=> next token is in FIRST(factor)
        bool in_first_factor() const
        {
            return lookahead_.type == symbol_type::left_paren ||
                lookahead_.type == symbol_type::number ||
                lookahead_.type == symbol_type::id;
        }
        
        // true <=> next token is in FOLLOW(factor)
        bool in_follow_factor() const
        {
            return in_follow_dice_roll();
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

        value_type factor()
        {
            if (lookahead_.type == symbol_type::left_paren)
            {
                eat(symbol_type::left_paren);
                auto result = expr();
                eat(symbol_type::right_paren);
                return result;
            }
            else if (lookahead_.type == symbol_type::number)
            {
                auto lexeme = lookahead_.lexeme;
                eat(lookahead_.type);
                if (lexeme.find('.') != std::string::npos)
                {
                    return make<type_double>(std::atof(lexeme.c_str()));
                }
                else 
                {
                    return make<type_int>(std::atoi(lexeme.c_str()));
                }
            }
            else if (lookahead_.type == symbol_type::id)
            {
                // parse a function call
                auto id = lookahead_;
                eat(symbol_type::id);
                eat(symbol_type::left_paren);
                auto args = param_list();
                eat(symbol_type::right_paren);
                
                return env_->call_var(id.lexeme, args.begin(), args.end());
            }
            
            error("Expected " + to_string(symbol_type::left_paren) + ", " + 
                to_string(symbol_type::number) + " or " + 
                to_string(symbol_type::id) +  ", got " + 
                to_string(lookahead_) + "."); 
            return make<type_int>(0);
        }

        // true <=> next token is in FIRST(param_list)
        bool in_first_param_list() const
        {
            return lookahead_.type == symbol_type::right_paren || 
                in_first_expr();
        }

        // true <=> next token is in FOLLOW(param_list)
        bool in_follow_param_list() const
        {
            return lookahead_.type == symbol_type::param_delim ||
                // FOLLOW(opt_params):
                lookahead_.type == symbol_type::right_paren; 
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

        std::vector<value_type> param_list()
        {
            std::vector<value_type> args;
            if (lookahead_.type == symbol_type::right_paren)
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
                    error("Invalid function parameter " + 
                        std::to_string(number));
                }
        
                if (lookahead_.type != symbol_type::param_delim)
                {
                    break;
                }
                eat(symbol_type::param_delim);
                ++number;
            }
            return args;
        }

        /** Read token of given type from the input.
         * If there is no such token, "add" it at current position.
         * New token is read into the lookahead_ variable.
         * @param expected type
         */
        void eat(symbol_type type)
        {
            if (lookahead_.type != type)
            {
                // report the error and "insert" the token 
                // (i.e. assume it's there) 
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