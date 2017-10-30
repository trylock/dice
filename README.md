# Dice expressions interpreter
Essential part of many tabletop games are random events. They are often introduced to the game in form of dice rolls. This program can help you to understand those dice rolls. You can give it a dice expression and it computes the probabilities and some statistics for you. It can even deal with some form of depedence using names (see examples).

## Examples
In all of these examples I assume that dice rolls are independent discrete (integer) random variables. 

- `2d6`: roll 2 six sided dice and add the results
- `(1d4)d6`: first, roll a 4 sided die. What you've rolled will determine how many times you'll roll a 6 sided die.
- `1d4 * 2`: roll a 4 sided die and multiply the result by 2
- `(1 + 2) * 3`: `9` (you don't have to use the dice operator)
- `(1d4)d(1d2*4)`: `1d4` will determine the number of dice and `1d2 * 4` will determine the number of dice sides
- `variance(2d6)`: compute the variance of `2d6` 
- `expectation(2d6)`: compute the expectation of `2d6`
- `5d100 < 50`: compute the indicator (i.e. random variable with bernoulli distribution) that `5d100` is less than 50 
- `3d6 == 6`: compute the indicator that `3d6` is exactly eqaul to 6
- `2d100 in [4, 32]`: compute the indicator that `2d100` is in the `[4, 32]` closed interval
- `var roll = 1d20; (roll in [10, 19]) * 2d8 + (roll == 20) * 4d8`: roll with a 20 sided die. If we get a number between 10 and 19 (including 10 and 19), roll `2d8`, if we get a 20, roll `4d8`. Otherwise it's 0. Notice the 2 occurances of `roll` are not independent. They are considered as *one* roll. 

## Pitfalls
- **independence**: The program works with random variables. **Each operation on them assumes their independence**. This assumption is quite limiting. Consider following expression: `2 * (1d20 == 19) + 3 * (1d20 == 20)`. Some misguided assumption clould be that this expression is equal to 2 if we roll a 19 and 3 if we roll a 20. This is *not* the case. Both subexpressions `1d20` are independent - they are completely different rolls. In this example, we can get 0, 2, 3 or 5. You can use variables to resolve this issue: `var X = 1d20; 2 * (X == 19) + 3 * (X == 20)`
- **int vs double**: The program can work only with integer random variables (that is, value of a variable can only be an integer). Therefore it is invalid to use operands on random variables in conjunction with floating point numbers. For example: `1d6 * 2.5` or even `1d6 + 2.0` are invalid.

## Operators
Operators in this list are sorted by precedence from lowest to highest. All operators are left-associative unless stated otherwise:

1. `=` (assign operator): non-associative
2. `<` (less than), `<=` (less than or equal), `!=` (not equal), `==` (equal), `>=` (greater than or equal), `>` (greater than), `in` (is in interval): all non-associative
3. `+` (addition), `-` (subtraction)
4. `*` (multiplication), `/` (division)
5. `-` (unary minus)
6. `d` or `D` (dice roll operator)

## Grammar
The program implements a predictive parser for it is easy to write it by hand. Original grammar is listed here. It is necessary to modify the grammar for a straightforward implementation (for example: get rid of the left recursion).

```
<stmts>        ::= <stmt>; <stmts> 
                 | <stmt>
                 | ""

<stmt>        ::= var <id> = <expr>
                | <expr>

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
                | <number>
                | <identifier>(<opt_params>)
                | <identifier>
                
<opt_params>  ::= <param_list> 
                | "" 
                
<param_list>  ::= <param_list>, <expr> 
                | <expr> 
```

Here are some non-trivial terminals and their regular expressions:
- `<number>`: `[0-9]+(\.[0-9]+)?`
- `<identifier>`: `[A-Za-z][A-Za-z0-9_]*`
- `<rel_op>`: `<|<=|==|!=|>=|>`

## Types
The program works with 3 basic types: `int` (signed integer), `double` (a floating point number) and `rand_var` (a discrete integer random variable). It can convert a value of type `integer` to all other types but not vice versa. Integers are implicitly converted to those types when needed (in function calls or when evaluating an operator).

## Project structure
The project is separated into 3 parts: a static library, simple CLI (command line interface) program and tests. The static library is linked with the CLI program and the tests.

### Requirements 
You will need `cmake` version 3.0 or later and a C++ compiler that supports C++14 (tested on gcc version 7.2.0). 

You'll need doxygen, gcov and lcov in order to build documentation and code coverage report respectively. They are not needed in order to build and run the CLI program and tests.

### Build
1. Create a build directory (say `build`) in the project root
2. In this directory run `cmake ..` (use the `-G` option to specify generator)
3. Compile (for example: run `make` if you've used `Unix Makefiles`)
