#include "environment.hpp"

namespace 
{
    using fn = dice::execution_context;

    // functions implementation

    fn::return_type dice_expectation(fn::context_type& context)
    {
        using namespace dice;
        return make<type_double>(
            context.arg<type_rand_var>(0)->data().expected_value()
        );
    }

    fn::return_type dice_variance(fn::context_type& context)
    {
        using namespace dice;
        return dice::make<type_double>(
            context.arg<type_rand_var>(0)->data().variance()
        );
    }

    fn::return_type dice_deviation(fn::context_type& context)
    {
        using namespace dice;
        return make<type_double>(
            context.arg<type_rand_var>(0)->data().deviation()
        );
    }

    fn::return_type dice_quantile(fn::context_type& context)
    {
        using namespace dice;
        auto prob = dice::clamp(
            context.arg<type_double>(1)->data(), 0.0, 1.0);
        return dice::make<type_int>(
            context.arg<type_rand_var>(0)->data().quantile(prob)
        );
    }

    // operator functions

    template<typename T>
    fn::return_type dice_add(fn::context_type& context)
    {
        auto& a = context.arg<T>(0)->data();
        auto& b = context.arg<T>(1)->data();
        a = a + b;
        return std::move(context.raw_arg(0));
    }

    template<typename T>
    fn::return_type dice_sub(fn::context_type& context)
    {
        auto& a = context.arg<T>(0)->data();
        auto& b = context.arg<T>(1)->data();
        a = a - b;
        return std::move(context.raw_arg(0));
    }

    template<typename T>
    fn::return_type dice_mult(fn::context_type& context)
    {
        auto& a = context.arg<T>(0)->data();
        auto& b = context.arg<T>(1)->data();
        a = a * b;
        return std::move(context.raw_arg(0));
    }

    template<typename T>
    fn::return_type dice_div(fn::context_type& context)
    {
        auto& a = context.arg<T>(0)->data();
        auto& b = context.arg<T>(1)->data();
        a = a / b;
        return std::move(context.raw_arg(0));
    }

    template<typename T>
    fn::return_type dice_unary_minus(fn::context_type& context)
    {
        auto& a = context.arg<T>(0)->data();
        a = -a;
        return std::move(context.raw_arg(0));
    }

    fn::return_type dice_roll_op(fn::context_type& context)
    {
        using namespace dice;
        auto& a = context.arg<type_rand_var>(0)->data();
        auto& b = context.arg<type_rand_var>(1)->data();
        a = roll(a, b);
        return std::move(context.raw_arg(0));
    }

    template<typename T>
    fn::return_type dice_rand_var_in(fn::context_type& context)
    {
        auto& var = context.arg<dice::type_rand_var>(0)->data();
        auto& lower_bound = context.arg<T>(1)->data();
        auto& upper_bound = context.arg<T>(2)->data();
        var = var.in(lower_bound, upper_bound);
        return std::move(context.raw_arg(0));
    }

    fn::return_type dice_less_than(fn::context_type& context)
    {
        using namespace dice;
        auto& a = context.arg<type_rand_var>(0)->data();
        auto& b = context.arg<type_rand_var>(1)->data();
        a = a.less_than(b);
        return std::move(context.raw_arg(0));
    }

    fn::return_type dice_less_than_or_equal(fn::context_type& context)
    {
        using namespace dice;
        auto& a = context.arg<type_rand_var>(0)->data();
        auto& b = context.arg<type_rand_var>(1)->data();
        a = a.less_than_or_equal(b);
        return std::move(context.raw_arg(0));
    }

    fn::return_type dice_equal(fn::context_type& context)
    {
        using namespace dice;
        auto& a = context.arg<type_rand_var>(0)->data();
        auto& b = context.arg<type_rand_var>(1)->data();
        a = a.equal(b);
        return std::move(context.raw_arg(0));
    }

    fn::return_type dice_not_equal(fn::context_type& context)
    {
        using namespace dice;
        auto& a = context.arg<type_rand_var>(0)->data();
        auto& b = context.arg<type_rand_var>(1)->data();
        a = a.not_equal(b);
        return std::move(context.raw_arg(0));
    }

    fn::return_type dice_greater_than(fn::context_type& context)
    {
        using namespace dice;
        auto& a = context.arg<type_rand_var>(0)->data();
        auto& b = context.arg<type_rand_var>(1)->data();
        a = a.greater_than(b);
        return std::move(context.raw_arg(0));
    }

    fn::return_type dice_greater_than_or_equal(fn::context_type& context)
    {
        using namespace dice;
        auto& a = context.arg<type_rand_var>(0)->data();
        auto& b = context.arg<type_rand_var>(1)->data();
        a = a.greater_than_or_equal(b);
        return std::move(context.raw_arg(0));
    }

    template<typename T>
    fn::return_type dice_min(fn::context_type& context)
    {
        using namespace dice;
        using namespace std;
        auto& a = context.arg<T>(0)->data();
        auto& b = context.arg<T>(1)->data();
        a = min(a, b);
        return std::move(context.raw_arg(0));
    }

    template<typename T>
    fn::return_type dice_max(fn::context_type& context)
    {
        using namespace dice;
        using namespace std;
        auto& a = context.arg<T>(0)->data();
        auto& b = context.arg<T>(1)->data();
        a = max(a, b);
        return std::move(context.raw_arg(0));
    }

    // generate a random number
    #ifndef DISABLE_RNG

    std::random_device dev;

    template<typename ProbType = double>
    struct dice_roll 
    {
        std::default_random_engine engine;
        std::uniform_real_distribution<ProbType> dist;

        dice_roll() : engine(dev()), dist(0, 1) {}

        fn::return_type operator()(fn::context_type& context)
        {
            using namespace dice;

            auto value = dist(engine);
            auto var = context.arg<type_rand_var>(0)->data()
                .to_random_variable(); 
            return make<type_int>(var.random_value(value));
        } 
    };

    #endif // DISABLE_RNG
}

// environment code

dice::environment::environment()
{
    // buildin operator functions
    add_function("+", {
        dice_add<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("+", {
        dice_add<type_double>, { type_double::id(), type_double::id() }
    });
    add_function("+", {
        dice_add<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("-", {
        dice_sub<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("-", {
        dice_sub<type_double>, { type_double::id(), type_double::id() }
    });
    add_function("-", {
        dice_sub<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("*", {
        dice_mult<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("*", {
        dice_mult<type_double>, { type_double::id(), type_double::id() }
    });
    add_function("*", {
        dice_mult<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("/", {
        dice_div<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("/", {
        dice_div<type_double>, { type_double::id(), type_double::id() }
    });
    add_function("/", {
        dice_div<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("unary-", {
        dice_unary_minus<type_int>, { type_int::id() }
    });
    add_function("unary-", {
        dice_unary_minus<type_double>, { type_double::id() }
    });
    add_function("unary-", {
        dice_unary_minus<type_rand_var>, { type_rand_var::id() }
    });
    add_function("roll_op", {
        dice_roll_op, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("in", {
        dice_rand_var_in<type_double>, { 
            type_rand_var::id(), 
            type_double::id(), 
            type_double::id() }
    });
    add_function("in", {
        dice_rand_var_in<type_int>, { 
            type_rand_var::id(), 
            type_int::id(), 
            type_int::id() }
    });
    add_function("<", {
        dice_less_than, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("<=", {
        dice_less_than_or_equal, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("==", {
        dice_equal, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("!=", {
        dice_not_equal, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function(">=", {
        dice_greater_than_or_equal, { 
            type_rand_var::id(), 
            type_rand_var::id() }
    });
    add_function(">", {
        dice_greater_than, { type_rand_var::id(), type_rand_var::id() }
    });

    // functions
    
    add_function("expectation", {
        dice_expectation, { type_rand_var::id() }
    });
    add_function("deviation", {
        dice_deviation, { type_rand_var::id() }
    });
    add_function("variance", {
        dice_variance, { type_rand_var::id() }
    });
    add_function("quantile", {
        dice_quantile, { type_rand_var::id(), type_double::id() }
    });

#ifndef DISABLE_RNG
    add_function("roll", {
        dice_roll<double>{}, { type_rand_var::id() }
    });
#endif // DISABLE_RNG

    // Compute minimum of 2 values
    add_function("min", {
        dice_min<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("min", {
        dice_min<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("min", {
        dice_min<type_double>, { type_double::id(), type_double::id() }
    });
    
    // Compute maximum of 2 values
    add_function("max", {
        dice_max<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("max", {
        dice_max<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("max", {
        dice_max<type_double>, { type_double::id(), type_double::id() }
    });
}

void dice::environment::add_function(
    const std::string& name, 
    function_definition function)
{
    auto result = functions_.insert(std::make_pair(
        name, 
        std::vector<function_definition>{}
    ));
    result.first->second.push_back(std::move(function));
}

void dice::environment::set_var(const std::string& name, value_type value)
{
    auto it = variables_.find(name);
    if (it != variables_.end())
    {
        it->second = std::move(value);
    }
    else 
    {
        variables_.insert(std::make_pair(
            name,
            std::move(value)
        ));
    }
}

dice::base_value* dice::environment::get_var(const std::string& name)
{
    auto it = variables_.find(name);
    if (it == variables_.end())
        return nullptr;
    return it->second.get();
}

const dice::base_value* dice::environment::get_var(const std::string& name) const
{
    auto it = variables_.find(name);
    if (it == variables_.end())
        return nullptr;
    return it->second.get();
}

fn::return_type dice::environment::call_prepared(
    const std::string& name, 
    execution_context& context)
{
    auto expected_argc = context.argc();

    // find functions in the function table
    auto it = functions_.find(name);
    if (it == functions_.end())
    {
        throw compiler_error("Function '" + name + "' was not defined.");
    }
    auto&& functions = it->second;

    // choose function with the lowest conversion cost
    function_definition* min_func = nullptr;
    auto min_cost = conversions::max_cost;
    for (auto&& function : functions)
    {
        if (function.argc() != expected_argc)
        {
            continue;
        }

        // calculate conversion cost for this function
        conversions::cost_type cost = 0;
        for (std::size_t i = 0; i < function.argc(); ++i)
        {
            auto to_type = function.arg_type(i);
            auto from_type = context.arg_type(i);
            auto conv_cost = conversions_.cost(from_type, to_type);
            if (conv_cost == conversions::max_cost)
            {
                cost = conv_cost;
                break;
            }
            cost += conv_cost;
        }

        // update minimal conversion function 
        if (cost < min_cost)
        {
            min_cost = cost;
            min_func = &function;
        }
    }

    // if there is no suitable conversion, the function call fails
    if (min_func == nullptr)
    {
        std::string error_message = "No matching function for: " ;
        error_message += name + "(";
        if (context.argc() > 0)
        {
            error_message += to_string(context.arg_type(0));
        }
        for (std::size_t i = 1; i < context.argc(); ++i)
        {
            error_message += ", " + to_string(context.arg_type(i));
        }
        error_message += ")";
        throw compiler_error(error_message);
    }

    // convert arguments
    for (std::size_t i = 0; i < context.argc(); ++i)
    {
        type_id to = min_func->arg_type(i);
        context.raw_arg(i) = conversions_.convert(to, 
            std::move(context.raw_arg(i)));
    }

    // execute it
    try
    {
        return (*min_func)(context);
    }
    catch (safe_int_error& error)
    {
        throw compiler_error{
            is_overflow_error(error) ? "Overflow" : "Division by Zero" };
    }
}