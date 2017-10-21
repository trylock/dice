#ifndef DICE_UTILS_HPP_
#define DICE_UTILS_HPP_

namespace dice 
{
    template<typename T, typename U>
    struct pair_hash
    {
        using value_type = std::pair<T, U>;

        std::hash<T> t_hash;
        std::hash<U> u_hash;

        std::size_t operator()(const value_type& value) const 
        {
            return t_hash(value.first) ^ ~u_hash(value.second); 
        }
    };
}

#endif // DICE_UTILS_HPP_