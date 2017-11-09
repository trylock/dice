#include "catch.hpp"
#include "decomposition.hpp"

using freq_list = dice::random_variable<int, double>::frequency_list;

TEST_CASE("Compute probability of indepedent random variables", "[decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> var_b(dice::constant_tag{}, 2);
    dice::decomposition<int, double> a{ var_a };
    dice::decomposition<int, double> b{ var_b };

    auto result = a + b;
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(3)->second == Approx(0.25));
    REQUIRE(prob.find(4)->second == Approx(0.25));
    REQUIRE(prob.find(5)->second == Approx(0.25));
    REQUIRE(prob.find(6)->second == Approx(0.25));
}

TEST_CASE("Compute probability of independent random variables", "[decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> var_b(dice::constant_tag{}, 2);
    dice::decomposition<int, double> a{ var_a };
    dice::decomposition<int, double> b{ var_b };

    auto result = a + b;
    auto var = result.to_random_variable();
    auto&& prob = var.probability();
    
    REQUIRE(prob.find(3)->second == Approx(0.25));
    REQUIRE(prob.find(4)->second == Approx(0.25));
    REQUIRE(prob.find(5)->second == Approx(0.25));
    REQUIRE(prob.find(6)->second == Approx(0.25));
}

TEST_CASE("Compute probability of dependent random variables", "[decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::decomposition<int, double> a{ var_a };
    a = a.compute_decomposition();

    auto result = a + a;
    auto var = result.to_random_variable();
    auto&& prob = var.probability();
    
    REQUIRE(prob.find(2)->second == Approx(0.25));
    REQUIRE(prob.find(4)->second == Approx(0.25));
    REQUIRE(prob.find(6)->second == Approx(0.25));
    REQUIRE(prob.find(8)->second == Approx(0.25));
}

TEST_CASE("Compute probability of mix of random variables", "[decomposition]")
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
    
    dice::decomposition<int, double> a{ var_a };
    dice::decomposition<int, double> b{ var_b };
    dice::decomposition<int, double> c{ var_c };
    dice::decomposition<int, double> one{ var_one };
    a = a.compute_decomposition();

    auto result = a * b + (one - a) * c;
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(1)->second == Approx(0.3 / 2 + 0.7 / 4));
    REQUIRE(prob.find(2)->second == Approx(0.3 / 2 + 0.7 / 4));
    REQUIRE(prob.find(3)->second == Approx(0.7 / 4));
    REQUIRE(prob.find(4)->second == Approx(0.7 / 4));
}

TEST_CASE("Compute probability of depedent indicators", "[decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> var_two(dice::constant_tag{}, 2);
    dice::random_variable<int, double> var_three(dice::constant_tag{}, 3);
    dice::decomposition<int, double> a{ var_a };
    dice::decomposition<int, double> two{ var_two };
    dice::decomposition<int, double> three{ var_three };
    a = a.compute_decomposition();

    auto result = a.less_than_or_equal(three) * a.equal(two);
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(1)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(0)->second == Approx(3 / 4.0));
}

TEST_CASE("Compute probability of X in [A, B]", "[decomposition]")
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

    dice::decomposition<int, double> a{ var_a };
    dice::decomposition<int, double> six{ var_six };
    a = a.compute_decomposition();

    auto result = a.in(4, 5) + a.equal(six);
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(1)->second == Approx(1 / 2.0));
    REQUIRE(prob.find(0)->second == Approx(1 / 2.0));
}

TEST_CASE("Compute probability of multiple depedent variables", "[decomposition][multiple_vars]")
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

    dice::decomposition<int, double> a{ var_a };
    dice::decomposition<int, double> b{ var_b };
    dice::decomposition<int, double> c{ var_c };
    a = a.compute_decomposition();
    b = b.compute_decomposition();
    c = c.compute_decomposition();

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

TEST_CASE("Compute negation of a random variable", "[decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 2),
        std::make_pair(3, 3),
        std::make_pair(4, 4),
    } };
    dice::decomposition<int, double> a{ var_a };
    a = a.compute_decomposition();
    
    auto result = -a;
    auto var = result.to_random_variable();
    auto&& prob = var.probability();

    REQUIRE(prob.find(-1)->second == Approx(1 / 10.0));
    REQUIRE(prob.find(-2)->second == Approx(2 / 10.0));
    REQUIRE(prob.find(-3)->second == Approx(3 / 10.0));
    REQUIRE(prob.find(-4)->second == Approx(4 / 10.0));
}

TEST_CASE("Compute maximum of dependent random variables", "[decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> var_one{ dice::constant_tag{}, 1 };
    dice::decomposition<int, double> a{ var_a };
    dice::decomposition<int, double> one{ var_one };
    a = a.compute_decomposition();

    auto result = max(a, a + one);
    auto var = result.to_random_variable();
    auto prob = var.probability();
    REQUIRE(prob.find(2)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(5)->second == Approx(1 / 4.0));
}

TEST_CASE("Compute minimum of dependent random variables", "[decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> var_one{ dice::constant_tag{}, 1 };
    dice::decomposition<int, double> a{ var_a };
    dice::decomposition<int, double> one{ var_one };
    a = a.compute_decomposition();

    auto result = min(a, a + one);
    auto var = result.to_random_variable();
    auto prob = var.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(2)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 4.0));
}

TEST_CASE("Compute expected value using decomposition", "[decomposition]")
{
    dice::decomposition<int, double> a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
    } };
    a = a.compute_decomposition();

    dice::decomposition<int, double> b{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
    } };
    b = b.compute_decomposition();
    
    auto result = a + b;
    auto expectation = result.expected_value();
    REQUIRE(expectation == Approx(3.5));
}

TEST_CASE("Compute variance using decomposition", "[decomposition]")
{
    dice::decomposition<int, double> a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
    } };
    a = a.compute_decomposition();

    dice::decomposition<int, double> b{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
    } };
    b = b.compute_decomposition();
    
    auto result = a + b;
    auto variance = result.variance();
    REQUIRE(variance == Approx(11 / 12.0));
}