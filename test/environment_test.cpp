#include "catch.hpp"
#include "environment.hpp"

TEST_CASE("Call operator + on integers", "[environment]")
{
    dice::environment env;
    auto result = env.call("+", dice::value<int>(1), dice::value<int>(2));
}