#include "parser.hpp"

const char dice::nonterminal<dice::nonterminal_type::stmts>::name[] = "statements";
const std::array<dice::symbol_type, 7> 
    dice::nonterminal<dice::nonterminal_type::stmts>::first{ {
        symbol_type::end,
        symbol_type::var,
        symbol_type::minus,
        symbol_type::left_paren,
        symbol_type::number,
        symbol_type::func_id,
        symbol_type::id
    } };
const std::array<dice::symbol_type, 1> 
    dice::nonterminal<dice::nonterminal_type::stmts>::follow{ {
        symbol_type::end,
    } };

const char dice::nonterminal<dice::nonterminal_type::stmt>::name[] = "statement";
const std::array<dice::symbol_type, 6> 
    dice::nonterminal<dice::nonterminal_type::stmt>::first{ {
        symbol_type::var,
        symbol_type::minus,
        symbol_type::left_paren,
        symbol_type::number,
        symbol_type::func_id,
        symbol_type::id
    } };
const std::array<dice::symbol_type, 2> 
    dice::nonterminal<dice::nonterminal_type::stmt>::follow{ {
        symbol_type::end,
        symbol_type::semicolon,
    } };

const char dice::nonterminal<dice::nonterminal_type::expr>::name[] = "expression";
const std::array<dice::symbol_type, 5>
    dice::nonterminal<dice::nonterminal_type::expr>::first{ {
        symbol_type::minus,
        symbol_type::left_paren,
        symbol_type::number,
        symbol_type::func_id,
        symbol_type::id
    } };
const std::array<dice::symbol_type, 4> 
    dice::nonterminal<dice::nonterminal_type::expr>::follow{ {
        symbol_type::end,
        symbol_type::semicolon,
        symbol_type::right_paren,
        symbol_type::param_delim,
    } };

const char dice::nonterminal<dice::nonterminal_type::add>::name[] = "addition";
const std::array<dice::symbol_type, 5> 
    dice::nonterminal<dice::nonterminal_type::add>::first{ {
        symbol_type::minus,
        symbol_type::left_paren,
        symbol_type::number,
        symbol_type::func_id,
        symbol_type::id
    } };
const std::array<dice::symbol_type, 9> 
    dice::nonterminal<dice::nonterminal_type::add>::follow{ {
        symbol_type::in,
        symbol_type::rel_op,
        symbol_type::param_delim,
        symbol_type::right_square_bracket,
        symbol_type::end,
        symbol_type::semicolon,
        symbol_type::right_paren,
        symbol_type::plus,
        symbol_type::minus,
    } };

const char dice::nonterminal<dice::nonterminal_type::mult>::name[] = "multiplication";
const std::array<dice::symbol_type, 5> 
    dice::nonterminal<dice::nonterminal_type::mult>::first{ {
        symbol_type::minus,
        symbol_type::left_paren,
        symbol_type::number,
        symbol_type::func_id,
        symbol_type::id
    } };
const std::array<dice::symbol_type, 11> 
    dice::nonterminal<dice::nonterminal_type::mult>::follow{ {
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
    } };

const char dice::nonterminal<dice::nonterminal_type::dice_roll>::name[] = "dice roll";
const std::array<dice::symbol_type, 5> 
    dice::nonterminal<dice::nonterminal_type::dice_roll>::first{ {
        symbol_type::minus,
        symbol_type::left_paren,
        symbol_type::number,
        symbol_type::func_id,
        symbol_type::id
    } };
const std::array<dice::symbol_type, 12> 
    dice::nonterminal<dice::nonterminal_type::dice_roll>::follow{ {
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
    } };

const char dice::nonterminal<dice::nonterminal_type::factor>::name[] = "factor";
const std::array<dice::symbol_type, 4> 
    dice::nonterminal<dice::nonterminal_type::factor>::first{ {
        symbol_type::left_paren,
        symbol_type::number,
        symbol_type::func_id,
        symbol_type::id
    } };
const std::array<dice::symbol_type, 12> 
    dice::nonterminal<dice::nonterminal_type::factor>::follow{ {
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
    } };

const char dice::nonterminal<dice::nonterminal_type::param_list>::name[] = "parameter list";
const std::array<dice::symbol_type, 6> 
    dice::nonterminal<dice::nonterminal_type::param_list>::first{ {
        symbol_type::minus,
        symbol_type::left_paren,
        symbol_type::number,
        symbol_type::id,
        symbol_type::func_id,
        symbol_type::right_paren,
    } };
const std::array<dice::symbol_type, 2> 
    dice::nonterminal<dice::nonterminal_type::param_list>::follow{ {
        symbol_type::param_delim,
        symbol_type::right_paren,
    } };
