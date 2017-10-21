#include "catch.hpp"
#include "random_variable.hpp"

TEST_CASE("Compute distribution of a single dice roll", "[random_variable]")
{
    dice::random_variable<int, double> num_dice{ dice::constant_tag{}, 1 };
    dice::random_variable<int, double> num_sides{ dice::constant_tag{}, 6 };

    auto dist = dice::random_variable<int, double>::roll(num_dice, num_sides);
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

    auto dist = dice::random_variable<int, double>::roll(num_dice, num_sides);
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

    auto dist = dice::random_variable<int, double>::roll(num_dice, num_sides);
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
    dice::random_variable<int, double> num_sides{ std::make_pair(4, 1), std::make_pair(2, 2) };

    auto dist = dice::random_variable<int, double>::roll(num_dice, num_sides);
    REQUIRE(dist.probability().find(2)->second == Approx(1.0 / (16 * 3) + 2.0 / (4 * 3)));
}

TEST_CASE("Roll throws an exception if number of sides or number of dice is non-positive", "[random_variable]")
{
    dice::random_variable<int, double> num_dice{ dice::constant_tag{}, 2 };
    dice::random_variable<int, double> num_sides{ std::make_pair(-4, 1), std::make_pair(4, 1) };
    
    using rv = dice::random_variable<int, double>;

    REQUIRE_THROWS_WITH(
        rv::roll(num_dice, num_sides),
        "Number of dice sides has to be a positive integer."
    );
    REQUIRE_THROWS_WITH(
        rv::roll(num_sides, num_dice),
        "Number of dice has to be a positive integer."
    );
}
