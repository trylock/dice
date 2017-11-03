#ifndef DICE_PARSER_HPP_
#define DICE_PARSER_HPP_

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "logger.hpp"
#include "value.hpp"
#include "lexer.hpp"
#include "symbols.hpp"
#include "interpreter.hpp"
#include "random_variable.hpp"

namespace dice
{
    enum nonterminal_type
    {
        stmts,
        stmt,
        expr,
        add,
        mult,
        dice_roll,
        factor,
        param_list
    };

    // FIRST and FOLLOW sets of nonterminals
    template<nonterminal_type type>
    class nonterminal 
    {
    };

    template<>
    class nonterminal<nonterminal_type::stmts>
    {
    public:
        static const char name[];
        static const std::array<symbol_type, 7> first;
        static const std::array<symbol_type, 1> follow;
    };

    template<>
    class nonterminal<nonterminal_type::stmt>
    {
    public:
        static const char name[];
        static const std::array<symbol_type, 6> first;
        static const std::array<symbol_type, 2> follow;
    };
    
    template<>
    class nonterminal<nonterminal_type::expr>
    {
    public:
        static const char name[];
        static const std::array<symbol_type, 5> first;
        static const std::array<symbol_type, 4> follow;
    };
    
    template<>
    class nonterminal<nonterminal_type::add>
    {
    public:
        static const char name[];
        static const std::array<symbol_type, 5> first;
        static const std::array<symbol_type, 9> follow;
    };
    
    template<>
    class nonterminal<nonterminal_type::mult>
    {
    public:
        static const char name[];
        static const std::array<symbol_type, 5> first;
        static const std::array<symbol_type, 11> follow;
    };
    
    template<>
    class nonterminal<nonterminal_type::dice_roll>
    {
    public:
        static const char name[];
        static const std::array<symbol_type, 5> first;
        static const std::array<symbol_type, 12> follow;
    };
    
    template<>
    class nonterminal<nonterminal_type::factor>
    {
    public:
        static const char name[];
        static const std::array<symbol_type, 4> first;
        static const std::array<symbol_type, 12> follow;
    };
    
    template<>
    class nonterminal<nonterminal_type::param_list>
    {
    public:
        static const char name[];
        static const std::array<symbol_type, 6> first;
        static const std::array<symbol_type, 2> follow;
    };

    // dice expression parser
    template<typename Lexer, typename Logger, typename Interpreter>
    class parser
    {
    public:
        using value_type = typename Interpreter::value_type;

        parser(Lexer* reader, Logger* log, Interpreter* inter) : 
            lexer_(reader), 
            log_(log),
            int_(inter) {}

        /** Parse expression provided by the lexer.
         * Operators are left associative unless stated otherwise.
         * List of operators (from lowest to highest precedence): 
         * -#  = (assignment, non-associative)
         * -#  <, <=, !=, ==, >=, >, in (relational operators, non-assosiative)
         * -#  + (add), - (subtract)
         * -#  * (multiply), / (divide) 
         * -#  - (unary minus)
         * -#  D|d (roll dice)
         * @return computed values
         */
        auto parse()
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
        Interpreter* int_;
        symbol lookahead_;

        std::vector<value_type> stmts()
        {
            std::vector<value_type> values;
            if (!check<nonterminal_type::stmt>() || 
                lookahead_.type == symbol_type::end)
            {
                return values;
            }
            
            values.push_back(stmt());
            for (;;)
            {
                if (lookahead_.type == symbol_type::semicolon)
                {
                    eat(lookahead_.type);
                    if (lookahead_.type == symbol_type::end)
                    {
                        break; // last statement can be followed by a ;
                    }

                    if (check<nonterminal_type::stmt>())
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
            return std::move(values);
        }

        value_type stmt()
        {
            if (lookahead_.type == symbol_type::var)
            {
                eat(lookahead_.type);
                auto id = lookahead_;
                eat(symbol_type::id);
                eat(symbol_type::assign);
                
                // parse value of the name
                int_->enter_assign();
                value_type value;
                if (check<nonterminal_type::expr>())
                {
                    value = expr();
                }
                else 
                {
                    error(
                        "Invalid expression. "
                        "Using the default value instead.");
                    value = int_->make_default();
                }
                int_->leave_assign();

                return int_->assign(id.lexeme, std::move(value));
            }
            return expr();
        }

        value_type expr()
        {
            auto left = add();
            if (lookahead_.type == symbol_type::in)
            {
                eat(symbol_type::in);
                eat(symbol_type::left_square_bracket);

                value_type lower_bound;
                value_type upper_bound;
        
                // parse lower bound of the interval
                if (check<nonterminal_type::add>())
                {
                    lower_bound = add();
                }
                else 
                {
                    error(
                        "Invalid operand for the lower bound of operator in");
                    lower_bound = int_->make_default();
                }

                eat(symbol_type::param_delim);
        
                // parse upper bound of the interval
                if (check<nonterminal_type::add>())
                {
                    upper_bound = add();
                }
                else 
                {
                    error(
                        "Invalid operand for the upper bound of operator in");
                    upper_bound = int_->make_default();
                }
        
                eat(symbol_type::right_square_bracket); 
        
                return int_->rel_in(
                    std::move(left), 
                    std::move(lower_bound), 
                    std::move(upper_bound));
            }
            else if (lookahead_.type == symbol_type::rel_op)
            {
                auto op = lookahead_;
                eat(symbol_type::rel_op);
                if (check<nonterminal_type::add>())
                {
                   return int_->rel_op(op.lexeme, std::move(left), add());
                }
                else 
                {
                    error("Invalid operand for " + to_string(op));
                }
            }
            return std::move(left);
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
                if (check<nonterminal_type::mult>())
                {
                    if (op == "+")
                        result = int_->add(std::move(result), mult());
                    else 
                        result = int_->sub(std::move(result), mult());
                }
                else // otherwise, ignore the operator
                {
                    error("Invalid operand for binary operator " + op);
                }
            }
            return std::move(result);
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
                if (check<nonterminal_type::dice_roll>())
                {
                    if (op == "*")
                        result = int_->mult(std::move(result), dice_roll());
                    else
                        result = int_->div(std::move(result), dice_roll());
                }
                else // otherwise, ignore the operator
                {
                    error("Invalid operand for binary operator " + op);
                }
            }
            return std::move(result);
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
                    if (check<nonterminal_type::factor>())
                    {
                        result = int_->roll(std::move(result), factor());
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
                result = int_->unary_minus(std::move(result));

            return std::move(result); 
        }

        value_type factor()
        {
            if (lookahead_.type == symbol_type::left_paren)
            {
                eat(symbol_type::left_paren);
                
                value_type result;
                if (check<nonterminal_type::expr>())
                {
                    result = expr();
                }
                else 
                {
                    error(
                        "Invalid expression. "
                        "Using the default value instead.");
                    result = int_->make_default();
                }
                
                eat(symbol_type::right_paren);
                return std::move(result);
            }
            else if (lookahead_.type == symbol_type::number)
            {
                auto lexeme = lookahead_.lexeme;
                eat(lookahead_.type);
                return int_->number(lexeme);
            }
            else if (lookahead_.type == symbol_type::func_id)
            {
                // function call
                auto id = lookahead_;
                eat(symbol_type::func_id);
                eat(symbol_type::left_paren);
                auto args = param_list();
                eat(symbol_type::right_paren);

                try
                {
                    return int_->call(id.lexeme, std::move(args));
                }
                catch (compiler_error& err)
                {
                    error(err.what());
                    return int_->make_default();
                }
            }
            else if (lookahead_.type == symbol_type::id)
            {
                // variable
                auto id = lookahead_;
                eat(symbol_type::id);
                try 
                {
                    return int_->variable(id.lexeme);
                }
                catch (compiler_error& err)
                {
                    error(err.what());
                    return int_->make_default();
                }
            }
            
            error("Expected " + to_string(symbol_type::left_paren) + ", " + 
                to_string(symbol_type::number) + ", " + 
                to_string(symbol_type::id) + " or " + 
                to_string(symbol_type::func_id) +  ", got " + 
                to_string(lookahead_) + "."); 
            return int_->make_default();
        }

        std::vector<value_type> param_list()
        {
            std::vector<value_type> args;
            if (lookahead_.type == symbol_type::right_paren)
            {
                // no arguments
                return std::move(args); 
            }
        
            std::size_t number = 0;
            for (;;)
            {
                if (check<nonterminal_type::expr>())
                {
                    args.emplace_back(expr());
                }
                else 
                {
                    error("Invalid function parameter " + 
                        std::to_string(number) + ". " +
                        "Using the default value instead.");
                    args.emplace_back(int_->make_default());
                }
        
                if (lookahead_.type != symbol_type::param_delim)
                {
                    break;
                }
                eat(symbol_type::param_delim);
                ++number;
            }
            return std::move(args);
        }

        /** Check whether the lookahead type is in the first set of 
         * nonterminal type.
         * @return true iff current lookahead is in the first set
         */
        template<nonterminal_type type>
        bool in_first() const 
        {
            for (auto&& value : nonterminal<type>::first)
            {
                if (value == lookahead_.type)
                {
                    return true;
                }
            }
            return false;
        }

        /** Check whether the lookahead type is in the follow set of
         * nonterminal type.
         * @return true iff current lookahead is in the follow set
         */
        template<nonterminal_type type>
        bool in_follow() const 
        {
            for (auto&& value : nonterminal<type>::follow)
            {
                if (value == lookahead_.type)
                {
                    return true;
                }
            }
            return false;
        }

        /** Skip terminals until we find something in the synch set.
         * Synchronizing set is union of the FIRST and FOLLOW sets.
         * @return true iff current lookahead is in the FIRST set
         *         (i.e. we can continue parsing)
         */
        template<nonterminal_type type>
        bool check()
        {
            while (!in_first<type>() && !in_follow<type>())
            {
                error("Invalid token at the beginning of " + 
                    std::string(nonterminal<type>::name) + ": " + 
                    to_string(lookahead_));
                eat(lookahead_.type);
            }
            return in_first<type>();
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
                symbol s{ type };
                int_->terminal(s);
            }
            else 
            {
                int_->terminal(lookahead_);
                lookahead_ = lexer_->read_token();
            }
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

    /** Create a parser without specifying template parameters.
     * @param lexer
     * @param logger
     * @param interpreter
     * @return parser with those parameters
     */
    template<typename Lexer, typename Logger, typename Interpreter>
    auto make_parser(Lexer* lex, Logger* log, Interpreter* interpret)
    {
        return parser<Lexer, Logger, Interpreter>{ lex, log, interpret };
    }
}

#endif // DICE_PARSER_HPP_