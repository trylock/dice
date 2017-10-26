#include "catch.hpp"
#include "random_variable_decomposition.hpp"

TEST_CASE("Compute probability of indepedent random variables", "[random_variable_decomposition]")
{
    dice::random_variable<int, double> var_a{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    };
    dice::random_variable<int, double> var_b(dice::constant_tag{}, 2);
    dice::random_variable_decomposition<int, double> a{ dice::independent_tag{}, &var_a };
    dice::random_variable_decomposition<int, double> b{ dice::independent_tag{}, &var_b };

    auto result = a.combine(b, [](auto&& value_a, auto&& value_b) 
    {
        return value_a + value_b;
    });
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(3)->second == Approx(0.25));
    REQUIRE(prob.find(4)->second == Approx(0.25));
    REQUIRE(prob.find(5)->second == Approx(0.25));
    REQUIRE(prob.find(6)->second == Approx(0.25));
}

TEST_CASE("Compute probability of independent random variables", "[random_variable_decomposition]")
{
    dice::random_variable<int, double> var_a{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    };
    dice::random_variable<int, double> var_b(dice::constant_tag{}, 2);
    dice::random_variable_decomposition<int, double> a{ dice::dependent_tag{}, &var_a };
    dice::random_variable_decomposition<int, double> b{ dice::independent_tag{}, &var_b };

    auto result = a.combine(b, [](auto&& value_a, auto&& value_b) 
    {
        return value_a + value_b;
    });
    auto var = result.to_random_variable();
    auto&& prob = var.probability();
    
    REQUIRE(prob.find(3)->second == Approx(0.25));
    REQUIRE(prob.find(4)->second == Approx(0.25));
    REQUIRE(prob.find(5)->second == Approx(0.25));
    REQUIRE(prob.find(6)->second == Approx(0.25));
}

TEST_CASE("Compute probability of dependent random variables", "[random_variable_decomposition]")
{
    dice::random_variable<int, double> var_a{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    };
    dice::random_variable_decomposition<int, double> a{ dice::dependent_tag{}, &var_a };
    dice::random_variable_decomposition<int, double> b{ dice::dependent_tag{}, &var_a };

    auto result = a.combine(b, [](auto&& value_a, auto&& value_b) 
    {
        return value_a + value_b;
    });
    auto var = result.to_random_variable();
    auto&& prob = var.probability();
    
    REQUIRE(prob.find(2)->second == Approx(0.25));
    REQUIRE(prob.find(4)->second == Approx(0.25));
    REQUIRE(prob.find(6)->second == Approx(0.25));
    REQUIRE(prob.find(8)->second == Approx(0.25));
}

TEST_CASE("Compute probability of mix of random variables", "[random_variable_decomposition]")
{

    dice::random_variable<int, double> var_a(dice::bernoulli_tag{}, 0.7);
    dice::random_variable<int, double> var_b{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    };
    dice::random_variable<int, double> var_c{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
    };
    dice::random_variable<int, double> var_one(dice::constant_tag{}, 1);
    
    dice::random_variable_decomposition<int, double> a{ dice::dependent_tag{}, &var_a };
    dice::random_variable_decomposition<int, double> b{ dice::independent_tag{}, &var_b };
    dice::random_variable_decomposition<int, double> c{ dice::independent_tag{}, &var_c };
    dice::random_variable_decomposition<int, double> one{ dice::independent_tag{}, &var_one };

    auto result = a * b + (one - a) * c;
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(1)->second == Approx(0.3 / 2 + 0.7 / 4));
    REQUIRE(prob.find(2)->second == Approx(0.3 / 2 + 0.7 / 4));
    REQUIRE(prob.find(3)->second == Approx(0.7 / 4));
    REQUIRE(prob.find(4)->second == Approx(0.7 / 4));
}