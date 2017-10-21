#include <memory>
#include <unordered_map>
#include <functional>

#include "utils.hpp"
#include "value.hpp"

namespace dice 
{
    // type conversions
    class conversions
    {
    public:
        using value_ptr = std::unique_ptr<base_value>;
        using key_type = std::pair<type_id, type_id>;
        using cost_type = std::size_t;

        struct conversion
        {
            std::function<value_ptr(value_ptr&&)> func;
            cost_type cost = 0;
        };

        // cost of an impossible conversion
        static const cost_type max_cost = std::numeric_limits<cost_type>::max();

        template<typename Func>
        void add_conversion(type_id from, type_id to, Func&& func, cost_type cost)
        {
            convert_.insert(std::make_pair(std::make_pair(from, to), conversion{
                std::forward<Func>(func),
                cost
            }));
        }

        /** Find conversion cost from type <from> to type <to>
         * @param id of the <from> type
         * @param id of the <to> type
         * @return cost of the conversion 
         *         0 => no conversion needed,
         *         max_cost => the conversion is not supported
         */
        cost_type cost(type_id from, type_id to) const;

        /** Convert value from type <from> to type <to>
         * @param id of the <from> type
         * @param id of the <to> type
         * @param value to convert
         * @return converted value
         */ 
        value_ptr convert(type_id from, type_id to, value_ptr&& value) const;
    private:
        std::unordered_map<key_type, conversion, pair_hash<type_id, type_id>> convert_;
    };
}