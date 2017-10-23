# Dice expressions interpreter
Essential part of many tabletop games are random events. They are often introduced to the game in form of dice rolls. This program can help you to understand those dice rolls. You can give it a dice expression and it computes the probabilities and some statistics for you.

## Examples
In all of these examples I assume that dice rolls are independent discrete (integer) random variables.

- `2d6`: roll with 2 six sided dice and add the results
- `(1d4)d6`: first, roll a 4 sided dice. What you roll will determine how many times you'll roll a six sided dice
- `1d4 * 2`: roll a 4 sided dice and multiply the result by 2
- `(1 + 2) * 3`: `9` (you don't have to use the dice operator)
- `(1d4)d(1d2*4)`: `1d4` will determine the number of dice and `1d2 * 4` will determine the number of dice sides
- `variance(2d6)`: compute the variance of `2d6` 
- `expectation(2d6)`: compute the expectation of `2d6`
- `5d100 < 50`: compute the indicator (i.e. random variable with bernoulli distribution) that `5d100` is less than 50 
- `3d6 == 6`: compute the indicator that `3d6` is exactly eqaul to 6
- `2d100 in [4, 32]`: compute the indicator that `2d100` is in the `[4, 32]` closed interval

## Operators
Operators in this list are sorted by increasing precedence. All operators are left-associative unless stated otherwise:

1. `<` (less than), `<=` (less than or equal), `!=` (not equal), `==` (equal), `>=` (greater than or equal), `>` (greater than), `in` (is in interval): all non-associative
2. `+` (addition), `-` (subtraction)
3. `*` (multiplication), `/` (division)
4. `-` (unary minus)
5. `d` or `D` (dice roll operator)

## Grammar
The program implements a predictive parser for it is easy to write it by hand. Original grammar is listed here. It is necessary to modify the grammar for a straightforward implementation (for example: get rid of the left recursion).

```
<expr>        ::= <add> in [<add>, <add>] 
                | <add> <rel_op> <add> 
                | <add>
                
<add>         ::= <add> + <mult> 
                | <add> - <mult> 
                | <mult>
                
<mult>        ::= <mult> * <minus> 
                | <mult> / <minus> 
                | <minus>
                
<minus>       ::= -<minus> 
                | <dice_roll>
               
<dice_roll>   ::= <dice_roll> d <factor> 
                | <factor>
                
<factor>      ::= (<expr>) 
                | <number_int>
                | <number_fp>
                | <identifier>(<opt_params>)
                
<opt_params>  ::= <param_list> 
                | "" 
                
<param_list>  ::= <param_list>, <expr> 
                | <expr> 
```

Here are some non-trivial terminals and their regular expressions:
- `<number_int>`: `[0-9]+`
- `<number_fp>`: `[0-9]+\.[0-9]+`
- `<identifier>`: `[A-Za-z][A-Za-z0-9_]*`
- `<rel_op>`: `<|<=|==|!=|>=|>`

## Types
The program works with 3 basic types: `int` (signed integer), `double` (a floating point number) and `rand_var` (a discrete random variable). It can convert a value of type `integer` to all other types but not vice versa. Integers are therefore implicitly converted to those types when needed (in function calls or when evaluating an operator).
