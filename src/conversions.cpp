#include "conversions.hpp"

dice::conversions::cost_type dice::conversions::cost(type_id from, type_id to) const
{
    if (from == to)
    {
        return 0;
    }

    auto it = convert_.find(std::make_pair(from, to));
    if (it == convert_.end())
    {
        return max_cost;
    }
    return it->second.cost;
}

dice::conversions::value_ptr dice::conversions::convert(type_id from, type_id to, value_ptr&& value) const 
{
    if (from == to)
    {
        return std::move(value);
    }

    auto it = convert_.find(std::make_pair(from, to));
    assert(it != convert_.end());
    return it->second.func(std::move(value));    
}
