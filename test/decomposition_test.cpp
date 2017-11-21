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

    REQUIRE(var.probability(3) == Approx(0.25));
    REQUIRE(var.probability(4) == Approx(0.25));
    REQUIRE(var.probability(5) == Approx(0.25));
    REQUIRE(var.probability(6) == Approx(0.25));
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
    
    REQUIRE(var.probability(3) == Approx(0.25));
    REQUIRE(var.probability(4) == Approx(0.25));
    REQUIRE(var.probability(5) == Approx(0.25));
    REQUIRE(var.probability(6) == Approx(0.25));
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
    
    REQUIRE(var.probability(2) == Approx(0.25));
    REQUIRE(var.probability(4) == Approx(0.25));
    REQUIRE(var.probability(6) == Approx(0.25));
    REQUIRE(var.probability(8) == Approx(0.25));
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

    REQUIRE(var.probability(1) == Approx(0.3 / 2 + 0.7 / 4));
    REQUIRE(var.probability(2) == Approx(0.3 / 2 + 0.7 / 4));
    REQUIRE(var.probability(3) == Approx(0.7 / 4));
    REQUIRE(var.probability(4) == Approx(0.7 / 4));
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

    REQUIRE(var.probability(1) == Approx(1 / 4.0));
    REQUIRE(var.probability(0) == Approx(3 / 4.0));
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

    REQUIRE(var.probability(1) == Approx(1 / 2.0));
    REQUIRE(var.probability(0) == Approx(1 / 2.0));
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
    auto a2 = a * a;
    auto b2 = b * b;
    auto c2 = c * c;
    auto a2b2 = a2 * b2;
    auto result = a2b2 * c2;
    auto var = result.to_random_variable();

    REQUIRE(var.probability(1) == Approx(1 / 24.0));
    REQUIRE(var.probability(4) == Approx(3 / 24.0));
    REQUIRE(var.probability(9) == Approx(2 / 24.0));
    REQUIRE(var.probability(16) == Approx(4 / 24.0));
    REQUIRE(var.probability(36) == Approx(4 / 24.0));
    REQUIRE(var.probability(64) == Approx(3 / 24.0));
    REQUIRE(var.probability(81) == Approx(1 / 24.0));
    REQUIRE(var.probability(144) == Approx(3 / 24.0));
    REQUIRE(var.probability(256) == Approx(1 / 24.0));
    REQUIRE(var.probability(324) == Approx(1 / 24.0));
    REQUIRE(var.probability(576) == Approx(1 / 24.0));
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

    REQUIRE(var.probability(-1) == Approx(1 / 10.0));
    REQUIRE(var.probability(-2) == Approx(2 / 10.0));
    REQUIRE(var.probability(-3) == Approx(3 / 10.0));
    REQUIRE(var.probability(-4) == Approx(4 / 10.0));
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

    REQUIRE(var.probability(2) == Approx(1 / 4.0));
    REQUIRE(var.probability(3) == Approx(1 / 4.0));
    REQUIRE(var.probability(4) == Approx(1 / 4.0));
    REQUIRE(var.probability(5) == Approx(1 / 4.0));
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

    REQUIRE(var.probability(1) == Approx(1 / 4.0));
    REQUIRE(var.probability(2) == Approx(1 / 4.0));
    REQUIRE(var.probability(3) == Approx(1 / 4.0));
    REQUIRE(var.probability(4) == Approx(1 / 4.0));
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

TEST_CASE("Compute distribution of dice roll with dependent variables", "[decomposition]")
{
    dice::decomposition<int, double> a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
    } };
    a = a.compute_decomposition();

    auto roll_a = roll(a, a);
    roll_a = roll_a.compute_decomposition();
    auto var = roll_a.to_random_variable();
    REQUIRE(var.probability(1) == Approx(1 / 2.0));
    REQUIRE(var.probability(2) == Approx(1 / 8.0));
    REQUIRE(var.probability(3) == Approx(1 / 4.0));
    REQUIRE(var.probability(4) == Approx(1 / 8.0));
}

TEST_CASE("Compute distribution of sum of dependent dice rolls", "[decomposition]")
{
    dice::decomposition<int, double> a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
    } };
    a = a.compute_decomposition();

    auto roll_a = roll(a, a).compute_decomposition();
    auto sum = roll_a + roll_a;
    auto var = sum.to_random_variable();
    REQUIRE(var.probability(2) == Approx(1 / 2.0));
    REQUIRE(var.probability(4) == Approx(1 / 8.0));
    REQUIRE(var.probability(6) == Approx(1 / 4.0));
    REQUIRE(var.probability(8) == Approx(1 / 8.0));
}

TEST_CASE("Decomposition adds values in correct order", "[decomposition]")
{
    dice::random_variable<int, double> var_a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 2)
    } };
    dice::random_variable<int, double> var_b{ freq_list{
        std::make_pair(3, 2),
        std::make_pair(4, 3)
    } };
    dice::random_variable<int, double> var_c{ freq_list{
        std::make_pair(5, 4),
        std::make_pair(6, 5)
    } };
    dice::decomposition<int, double> a{ var_a };
    a = a.compute_decomposition();

    // The value is B if A = 1. Otherwise, it is C. Thus we have to store 
    // variables B and C according to the order of values in A. This method
    // is therefore not a part of the API. It's only purpose is to create a 
    // simple test data in this test case.
    if (var_a.begin()->first == 1)
    {
        a.variables_internal()[0] = var_c;
        a.variables_internal()[1] = var_b;
    }
    else
    {
        a.variables_internal()[0] = var_b;
        a.variables_internal()[1] = var_c;
    }
    a = a.compute_decomposition();

    auto var = a.to_random_variable();
    REQUIRE(var.probability(3) == Approx(4 / 15.0));
    REQUIRE(var.probability(4) == Approx(2 / 5.0));
    REQUIRE(var.probability(5) == Approx(4 / 27.0));
    REQUIRE(var.probability(6) == Approx(5 / 27.0));
}

TEST_CASE("Convert an empty decomposition to random variable", "[decomposition]")
{
    dice::decomposition<int, double> value;
    auto var = value.to_random_variable();
    REQUIRE(var.empty());
}

TEST_CASE("Convert decomposition with no dependencies to random varaible", "[decomposition]")
{
    dice::random_variable<int, double> test{ dice::bernoulli_tag{}, 0.8 };
    dice::decomposition<int, double> value{ test };
    auto var = value.to_random_variable();
    REQUIRE(var.probability(0) == Approx(0.2));
    REQUIRE(var.probability(1) == Approx(0.8));
}