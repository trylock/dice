#include "conversions.hpp"

const dice::conversions::cost_type dice::conversions::max_cost = 
    std::numeric_limits<dice::conversions::cost_type>::max();

dice::conversions::cost_type dice::conversions::cost(
    type_id from, 
    type_id to) const
{
    if (from == to)
    {
        return 0;
    }

    if (from == dice::type_int::id())
    {
        return 1;
    }
    return max_cost;
}

dice::conversions::value_type dice::conversions::convert(
    type_id to, 
    value_type&& value) const 
{
    auto from = value->type();
    if (from == to)
    {
        return std::move(value);
    }

    conversion_visitor conversion{ to };
    value->accept(&conversion);
    return conversion.value();
}
