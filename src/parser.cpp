#include "parser.hpp"

constexpr char dice::nonterminal<dice::nonterminal_type::stmts>::name[];
constexpr std::array<dice::symbol_type, 6> dice::nonterminal<dice::nonterminal_type::stmts>::first;
constexpr std::array<dice::symbol_type, 1> dice::nonterminal<dice::nonterminal_type::stmts>::follow;

constexpr char dice::nonterminal<dice::nonterminal_type::stmt>::name[];
constexpr std::array<dice::symbol_type, 5> dice::nonterminal<dice::nonterminal_type::stmt>::first;
constexpr std::array<dice::symbol_type, 2> dice::nonterminal<dice::nonterminal_type::stmt>::follow;

constexpr char dice::nonterminal<dice::nonterminal_type::expr>::name[];
constexpr std::array<dice::symbol_type, 4> dice::nonterminal<dice::nonterminal_type::expr>::first;
constexpr std::array<dice::symbol_type, 4> dice::nonterminal<dice::nonterminal_type::expr>::follow;

constexpr char dice::nonterminal<dice::nonterminal_type::add>::name[];
constexpr std::array<dice::symbol_type, 4> dice::nonterminal<dice::nonterminal_type::add>::first;
constexpr std::array<dice::symbol_type, 9> dice::nonterminal<dice::nonterminal_type::add>::follow;

constexpr char dice::nonterminal<dice::nonterminal_type::mult>::name[];
constexpr std::array<dice::symbol_type, 4> dice::nonterminal<dice::nonterminal_type::mult>::first;
constexpr std::array<dice::symbol_type, 11> dice::nonterminal<dice::nonterminal_type::mult>::follow;

constexpr char dice::nonterminal<dice::nonterminal_type::dice_roll>::name[];
constexpr std::array<dice::symbol_type, 4> dice::nonterminal<dice::nonterminal_type::dice_roll>::first;
constexpr std::array<dice::symbol_type, 12> dice::nonterminal<dice::nonterminal_type::dice_roll>::follow;

constexpr char dice::nonterminal<dice::nonterminal_type::factor>::name[];
constexpr std::array<dice::symbol_type, 3> dice::nonterminal<dice::nonterminal_type::factor>::first;
constexpr std::array<dice::symbol_type, 12> dice::nonterminal<dice::nonterminal_type::factor>::follow;

constexpr char dice::nonterminal<dice::nonterminal_type::param_list>::name[];
constexpr std::array<dice::symbol_type, 5> dice::nonterminal<dice::nonterminal_type::param_list>::first;
constexpr std::array<dice::symbol_type, 2> dice::nonterminal<dice::nonterminal_type::param_list>::follow;
