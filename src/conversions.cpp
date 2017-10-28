#include "conversions.hpp"

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

dice::conversions::value_ptr dice::conversions::convert(
    type_id from, 
    type_id to, 
    value_ptr&& value) const 
{
    if (from == to)
    {
        return std::move(value);
    }

    if (from != dice::type_int::id())
    {
        return nullptr; // conversion is not possible
    }
    
    auto int_value = dynamic_cast<type_int&>(*value);
    if (to == dice::type_double::id())
    {
        return make<type_double>(static_cast<double>(int_value.data()));
    }
    else if (to == dice::type_rand_var::id())
    {
        return make<type_rand_var>(constant_tag{}, int_value.data());
    }
    return nullptr;
}
