#include "catch.hpp"
#include "random_variable_decomposition.hpp"

using freq_list = dice::random_variable<int, double>::freq_list;

TEST_CASE("Compute probability of indepedent random variables", "[random_variable_decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
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
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
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
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
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
    dice::random_variable<int, double> var_b{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> var_c{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
    } };
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

TEST_CASE("Compute probability of depedent indicators", "[random_variable_decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> var_two(dice::constant_tag{}, 2);
    dice::random_variable<int, double> var_three(dice::constant_tag{}, 3);
    dice::random_variable_decomposition<int, double> a{ dice::dependent_tag{}, &var_a };
    dice::random_variable_decomposition<int, double> two{ dice::independent_tag{}, &var_two };
    dice::random_variable_decomposition<int, double> three{ dice::independent_tag{}, &var_three };
    auto result = a.less_than_or_equal(three) * a.equal(two);
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(1)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(0)->second == Approx(3 / 4.0));
}

TEST_CASE("Compute probability of X in [A, B]", "[random_variable_decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
        std::make_pair(5, 1),
        std::make_pair(6, 1),
    } };
    dice::random_variable<int, double> var_six(dice::constant_tag{}, 6);

    dice::random_variable_decomposition<int, double> a{ dice::dependent_tag{}, &var_a };
    dice::random_variable_decomposition<int, double> six{ dice::independent_tag{}, &var_six };

    auto result = a.in(4, 5) + a.equal(six);
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(1)->second == Approx(1 / 2.0));
    REQUIRE(prob.find(0)->second == Approx(1 / 2.0));
}

TEST_CASE("Compute probability of multiple depedent variables", "[random_variable_decomposition][multiple_vars]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> var_b{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
    } };
    dice::random_variable<int, double> var_c{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
    } };

    dice::random_variable_decomposition<int, double> a{ dice::dependent_tag{}, &var_a };
    dice::random_variable_decomposition<int, double> b{ dice::dependent_tag{}, &var_b };
    dice::random_variable_decomposition<int, double> c{ dice::dependent_tag{}, &var_c };

    // notice: A, B and B, C and A, C and A, B, C are still indepednet
    auto result = a * a * b * b * c * c;
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(1)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(4)->second == Approx(3 / 24.0));
    REQUIRE(prob.find(9)->second == Approx(2 / 24.0));
    REQUIRE(prob.find(16)->second == Approx(4 / 24.0));
    REQUIRE(prob.find(36)->second == Approx(4 / 24.0));
    REQUIRE(prob.find(64)->second == Approx(3 / 24.0));
    REQUIRE(prob.find(81)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(144)->second == Approx(3 / 24.0));
    REQUIRE(prob.find(256)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(324)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(576)->second == Approx(1 / 24.0));
}

TEST_CASE("Compute negation of a random variable", "[random_variable_decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 2),
        std::make_pair(3, 3),
        std::make_pair(4, 4),
    } };
    dice::random_variable_decomposition<int, double> a{ dice::dependent_tag{}, &var_a };

    auto result = -a;
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(-1)->second == Approx(1 / 10.0));
    REQUIRE(prob.find(-2)->second == Approx(2 / 10.0));
    REQUIRE(prob.find(-3)->second == Approx(3 / 10.0));
    REQUIRE(prob.find(-4)->second == Approx(4 / 10.0));
}

TEST_CASE("Compute maximum of dependent random variables", "[random_variable_decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> var_one{ dice::constant_tag{}, 1 };
    dice::random_variable_decomposition<int, double> a{ dice::dependent_tag{}, &var_a };
    dice::random_variable_decomposition<int, double> one{ dice::independent_tag{}, &var_one };

    auto result = max(a, a + one);
    auto var = result.to_random_variable();
    auto prob = var.probability();
    REQUIRE(prob.find(2)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(5)->second == Approx(1 / 4.0));
}

TEST_CASE("Compute minimum of dependent random variables", "[random_variable_decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> var_one{ dice::constant_tag{}, 1 };
    dice::random_variable_decomposition<int, double> a{ dice::dependent_tag{}, &var_a };
    dice::random_variable_decomposition<int, double> one{ dice::independent_tag{}, &var_one };

    auto result = min(a, a + one);
    auto var = result.to_random_variable();
    auto prob = var.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(2)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 4.0));
}