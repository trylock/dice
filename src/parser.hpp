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
        static constexpr char name[] = "statements";
        static constexpr std::array<symbol_type, 6> first{{
            symbol_type::end,
            symbol_type::var,
            symbol_type::minus,
            symbol_type::left_paren,
            symbol_type::number,
            symbol_type::id
        }};
        static constexpr std::array<symbol_type, 1> follow{{
            symbol_type::end,
        }};
    };

    template<>
    class nonterminal<nonterminal_type::stmt>
    {
    public:
        static constexpr char name[] = "statement";
        static constexpr std::array<symbol_type, 5> first{{
            symbol_type::var,
            symbol_type::minus,
            symbol_type::left_paren,
            symbol_type::number,
            symbol_type::id
        }};
        static constexpr std::array<symbol_type, 2> follow{{
            symbol_type::end,
            symbol_type::semicolon,
        }};
    };
    
    template<>
    class nonterminal<nonterminal_type::expr>
    {
    public:
        static constexpr char name[] = "expression";
        static constexpr std::array<symbol_type, 4> first{{
            symbol_type::minus,
            symbol_type::left_paren,
            symbol_type::number,
            symbol_type::id
        }};
        static constexpr std::array<symbol_type, 3> follow{{
            symbol_type::end,
            symbol_type::semicolon,
            symbol_type::right_paren,
        }};
    };
    
    template<>
    class nonterminal<nonterminal_type::add>
    {
    public:
        static constexpr char name[] = "addition";
        static constexpr std::array<symbol_type, 4> first{{
            symbol_type::minus,
            symbol_type::left_paren,
            symbol_type::number,
            symbol_type::id
        }};
        static constexpr std::array<symbol_type, 9> follow{{
            symbol_type::in,
            symbol_type::rel_op,
            symbol_type::param_delim,
            symbol_type::right_square_bracket,
            symbol_type::end,
            symbol_type::semicolon,
            symbol_type::right_paren,
            symbol_type::plus,
            symbol_type::minus,
        }};
    };
    
    template<>
    class nonterminal<nonterminal_type::mult>
    {
    public:
        static constexpr char name[] = "multiplication";
        static constexpr std::array<symbol_type, 4> first{{
            symbol_type::minus,
            symbol_type::left_paren,
            symbol_type::number,
            symbol_type::id
        }};
        static constexpr std::array<symbol_type, 11> follow{{
            symbol_type::in,
            symbol_type::rel_op,
            symbol_type::param_delim,
            symbol_type::right_square_bracket,
            symbol_type::end,
            symbol_type::semicolon,
            symbol_type::right_paren,
            symbol_type::plus,
            symbol_type::minus,
            symbol_type::times,
            symbol_type::divide,
        }};
    };
    
    template<>
    class nonterminal<nonterminal_type::dice_roll>
    {
    public:
        static constexpr char name[] = "dice roll";
        static constexpr std::array<symbol_type, 4> first{{
            symbol_type::minus,
            symbol_type::left_paren,
            symbol_type::number,
            symbol_type::id
        }};
        static constexpr std::array<symbol_type, 12> follow{{
            symbol_type::in,
            symbol_type::rel_op,
            symbol_type::param_delim,
            symbol_type::right_square_bracket,
            symbol_type::end,
            symbol_type::semicolon,
            symbol_type::right_paren,
            symbol_type::plus,
            symbol_type::minus,
            symbol_type::times,
            symbol_type::divide,
            symbol_type::roll_op,
        }};
    };
    
    template<>
    class nonterminal<nonterminal_type::factor>
    {
    public:
        static constexpr char name[] = "factor";
        static constexpr std::array<symbol_type, 3> first{{
            symbol_type::left_paren,
            symbol_type::number,
            symbol_type::id
        }};
        static constexpr std::array<symbol_type, 12> follow{{
            symbol_type::in,
            symbol_type::rel_op,
            symbol_type::param_delim,
            symbol_type::right_square_bracket,
            symbol_type::end,
            symbol_type::semicolon,
            symbol_type::right_paren,
            symbol_type::plus,
            symbol_type::minus,
            symbol_type::times,
            symbol_type::divide,
            symbol_type::roll_op,
        }};
    };
    
    template<>
    class nonterminal<nonterminal_type::param_list>
    {
    public:
        static constexpr char name[] = "parameter list";
        static constexpr std::array<symbol_type, 5> first{{
            symbol_type::minus,
            symbol_type::left_paren,
            symbol_type::number,
            symbol_type::id,
            symbol_type::right_paren,
        }};
        static constexpr std::array<symbol_type, 2> follow{{
            symbol_type::param_delim,
            symbol_type::right_paren,
        }};
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
        Interpreter* int_;
        symbol lookahead_;
        bool is_definition_ = false;

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
            return values;
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
                is_definition_ = true;
                value_type value;
                if (check<nonterminal_type::expr>())
                {
                    value = expr();
                }
                else 
                {
                    error("Invalid expression.");
                    value = int_->make_default();
                }
                is_definition_ = false;

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
        
                // parse lower bound of the interval
                if (!check<nonterminal_type::add>())
                {
                    error(
                        "Invalid operand for the lower bound of operator in");
                    return left;
                }
                auto lower_bound = add();
        
                eat(symbol_type::param_delim);
        
                // parse upper bound of the interval
                if (!check<nonterminal_type::add>())
                {
                    error(
                        "Invalid operand for the upper bound of operator in");
                    return left;
                }
                auto upper_bound = add();
        
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
            return left;
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
            return result;
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
            return result;
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
            // TODO: should we call the unary minus multiple times?
            if (count % 2 != 0)
                result = int_->unary_minus(std::move(result));
            return result; 
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
                    error("Invalid expression.");
                    result = int_->make_default();
                }
                
                eat(symbol_type::right_paren);
                return result;
            }
            else if (lookahead_.type == symbol_type::number)
            {
                auto lexeme = lookahead_.lexeme;
                eat(lookahead_.type);
                return int_->number(lexeme);
            }
            else if (lookahead_.type == symbol_type::id)
            {
                auto id = lookahead_;
                eat(symbol_type::id);

                if (lookahead_.type == symbol_type::left_paren) 
                {
                    // function call
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
                else // variable 
                {
                    if (is_definition_)
                    {
                        error("Using names of random variables in name "
                            "definition is supported only partially and " 
                            "may lead to incorrect results.");
                    }
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
            }
            
            error("Expected " + to_string(symbol_type::left_paren) + ", " + 
                to_string(symbol_type::number) + " or " + 
                to_string(symbol_type::id) +  ", got " + 
                to_string(lookahead_) + "."); 
            return int_->make_default();
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
                if (check<nonterminal_type::expr>())
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
            }
            else 
            {
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

    /** Create a parser without specifying template parameter.
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