#include "catch.hpp"
#include "random_variable.hpp"

using freq_list = dice::random_variable<int, double>::freq_list;

TEST_CASE("Compute distribution of a single dice roll", "[random_variable]")
{
    dice::random_variable<int, double> num_dice{ dice::constant_tag{}, 1 };
    dice::random_variable<int, double> num_sides{ dice::constant_tag{}, 6 };

    auto dist = roll(num_dice, num_sides);
    REQUIRE(dist.probability().size() == 6);
    REQUIRE(dist.probability().find(1)->second == Approx(1 / 6.0));
    REQUIRE(dist.probability().find(2)->second == Approx(1 / 6.0));
    REQUIRE(dist.probability().find(3)->second == Approx(1 / 6.0));
    REQUIRE(dist.probability().find(4)->second == Approx(1 / 6.0));
    REQUIRE(dist.probability().find(5)->second == Approx(1 / 6.0));
    REQUIRE(dist.probability().find(6)->second == Approx(1 / 6.0));
}

TEST_CASE("Compute distribution of 2d6", "[random_variable]")
{
    dice::random_variable<int, double> num_dice{ dice::constant_tag{}, 2 };
    dice::random_variable<int, double> num_sides{ dice::constant_tag{}, 6 };

    auto dist = roll(num_dice, num_sides);
    REQUIRE(dist.probability().size() == 11);
    REQUIRE(dist.probability().find(2)->second == Approx(1 / 36.0));
    REQUIRE(dist.probability().find(3)->second == Approx(2 / 36.0));
    REQUIRE(dist.probability().find(4)->second == Approx(3 / 36.0));
    REQUIRE(dist.probability().find(5)->second == Approx(4 / 36.0));
    REQUIRE(dist.probability().find(6)->second == Approx(5 / 36.0));
    REQUIRE(dist.probability().find(7)->second == Approx(6 / 36.0));
    REQUIRE(dist.probability().find(8)->second == Approx(5 / 36.0));
    REQUIRE(dist.probability().find(9)->second == Approx(4 / 36.0));
    REQUIRE(dist.probability().find(10)->second == Approx(3 / 36.0));
    REQUIRE(dist.probability().find(11)->second == Approx(2 / 36.0));
    REQUIRE(dist.probability().find(12)->second == Approx(1 / 36.0));
}

TEST_CASE("Compute distribution of 4d4", "[random_variable]")
{
    dice::random_variable<int, double> num_dice{ dice::constant_tag{}, 4 };
    dice::random_variable<int, double> num_sides{ dice::constant_tag{}, 4 };

    auto dist = roll(num_dice, num_sides);
    REQUIRE(dist.probability().size() == 13);
    REQUIRE(dist.probability().find(4)->second == Approx(1 / 256.0));
    REQUIRE(dist.probability().find(5)->second == Approx(4 / 256.0));
    REQUIRE(dist.probability().find(6)->second == Approx(10 / 256.0));
    REQUIRE(dist.probability().find(7)->second == Approx(20 / 256.0));
    REQUIRE(dist.probability().find(8)->second == Approx(31 / 256.0));
    REQUIRE(dist.probability().find(9)->second == Approx(40 / 256.0));
    REQUIRE(dist.probability().find(10)->second == Approx(44 / 256.0));
    REQUIRE(dist.probability().find(11)->second == Approx(40 / 256.0));
    REQUIRE(dist.probability().find(12)->second == Approx(31 / 256.0));
    REQUIRE(dist.probability().find(13)->second == Approx(20 / 256.0));
    REQUIRE(dist.probability().find(14)->second == Approx(10 / 256.0));
    REQUIRE(dist.probability().find(15)->second == Approx(4 / 256.0));
    REQUIRE(dist.probability().find(16)->second == Approx(1 / 256.0));
}

TEST_CASE("Compute distribution of XdY where X and Y are random variables", "[random_variable]")
{
    dice::random_variable<int, double> num_dice{ dice::constant_tag{}, 2 };
    dice::random_variable<int, double> num_sides{ 
        freq_list{ std::make_pair(4, 1), std::make_pair(2, 2) } 
    };

    auto dist = roll(num_dice, num_sides);
    REQUIRE(dist.probability().find(2)->second == Approx(1.0 / (16 * 3) + 2.0 / (4 * 3)));
}

TEST_CASE("Roll throws an exception if number of sides or number of dice is non-positive", "[random_variable]")
{
    dice::random_variable<int, double> num_dice{ dice::constant_tag{}, 2 };
    dice::random_variable<int, double> num_sides{ 
        freq_list { std::make_pair(-4, 1), std::make_pair(4, 1) } 
    };
    
    REQUIRE_THROWS_WITH(
        roll(num_dice, num_sides),
        "Number of dice sides has to be a positive integer."
    );
    REQUIRE_THROWS_WITH(
        roll(num_sides, num_dice),
        "Number of dice has to be a positive integer."
    );
}

TEST_CASE("Random variable with exactly 1 value is constant", "[dice]")
{
    using rv = dice::random_variable<int, double>;
    REQUIRE(!rv().is_constant());

    REQUIRE(rv(dice::constant_tag{}, 5).is_constant());
    REQUIRE(rv(dice::constant_tag{}, 5).value() == 5);

    REQUIRE(!rv(dice::bernoulli_tag{}, 0.5).is_constant());

    REQUIRE(rv(dice::bernoulli_tag{}, 1).is_constant());
    REQUIRE(rv(dice::bernoulli_tag{}, 1).value() == 1);
    
    REQUIRE(rv(dice::bernoulli_tag{}, 0).is_constant());
    REQUIRE(rv(dice::bernoulli_tag{}, 0).value() == 0);

    REQUIRE(rv({ std::make_pair(5, 1) }).is_constant());
    REQUIRE(rv({ std::make_pair(5, 1) }).value() == 5);

    REQUIRE(!rv({ std::make_pair(5, 1), std::make_pair(1, 1) }).is_constant());
    REQUIRE(!rv().is_constant());

    REQUIRE(rv({ std::make_pair(4, 1), std::make_pair(5, 0) }).is_constant());
    REQUIRE(rv({ std::make_pair(4, 1), std::make_pair(5, 0) }).value() == 4);
}

TEST_CASE("Add random variables", "[dice]")
{
    dice::random_variable<int, double> a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> b{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
        std::make_pair(5, 1),
        std::make_pair(6, 1),
    } };
    auto result = a + b;
    auto&& prob = result.probability();
    REQUIRE(prob.find(2)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(3)->second == Approx(2 / 24.0));
    REQUIRE(prob.find(4)->second == Approx(3 / 24.0));
    REQUIRE(prob.find(5)->second == Approx(4 / 24.0));
    REQUIRE(prob.find(6)->second == Approx(4 / 24.0));
    REQUIRE(prob.find(7)->second == Approx(4 / 24.0));
    REQUIRE(prob.find(8)->second == Approx(3 / 24.0));
    REQUIRE(prob.find(9)->second == Approx(2 / 24.0));
    REQUIRE(prob.find(10)->second == Approx(1 / 24.0));
}

TEST_CASE("Subtract random variables", "[dice]")
{
    dice::random_variable<int, double> a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> b{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
        std::make_pair(5, 1),
        std::make_pair(6, 1),
    } };
    auto result = a - b;
    auto&& prob = result.probability();
    REQUIRE(prob.find(-5)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(-4)->second == Approx(2 / 24.0));
    REQUIRE(prob.find(-3)->second == Approx(3 / 24.0));
    REQUIRE(prob.find(-2)->second == Approx(4 / 24.0));
    REQUIRE(prob.find(-1)->second == Approx(4 / 24.0));
    REQUIRE(prob.find(0)->second == Approx(4 / 24.0));
    REQUIRE(prob.find(1)->second == Approx(3 / 24.0));
    REQUIRE(prob.find(2)->second == Approx(2 / 24.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 24.0));
}

TEST_CASE("Multiply random variables", "[dice]")
{
    dice::random_variable<int, double> a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> b{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
        std::make_pair(5, 1),
        std::make_pair(6, 1),
    } };
    auto result = a * b;
    auto&& prob = result.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(2)->second == Approx(2 / 24.0));
    REQUIRE(prob.find(3)->second == Approx(2 / 24.0));
    REQUIRE(prob.find(4)->second == Approx(3 / 24.0));
    REQUIRE(prob.find(5)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(6)->second == Approx(3 / 24.0));
    REQUIRE(prob.find(8)->second == Approx(2 / 24.0));
    REQUIRE(prob.find(9)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(10)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(12)->second == Approx(3 / 24.0));
    REQUIRE(prob.find(15)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(16)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(18)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(20)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(24)->second == Approx(1 / 24.0));
}

TEST_CASE("Divide random variables", "[dice]")
{
    dice::random_variable<int, double> a{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
    } };
    dice::random_variable<int, double> b{ freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
        std::make_pair(5, 1),
        std::make_pair(6, 1),
    } };
    auto result = b / a;
    auto&& prob = result.probability();
    REQUIRE(prob.find(0)->second == Approx(6 / 24.0));
    REQUIRE(prob.find(1)->second == Approx(9 / 24.0));
    REQUIRE(prob.find(2)->second == Approx(4 / 24.0));
    REQUIRE(prob.find(3)->second == Approx(2 / 24.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(5)->second == Approx(1 / 24.0));
    REQUIRE(prob.find(6)->second == Approx(1 / 24.0));
}

TEST_CASE("If number of sides or number of dice is an impossible event, the roll is an impossible as well", "[dice]")
{
    dice::random_variable<int, double> impossible{};
    dice::random_variable<int, double> constant(dice::constant_tag{}, 5);
    auto inv_dice = roll(impossible, constant);
    auto inv_sides = roll(constant, impossible);

    REQUIRE(inv_dice.probability().empty());
    REQUIRE(inv_sides.probability().empty());
}
