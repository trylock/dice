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
        using cost_type = std::size_t;

        // cost of an impossible conversion
        static const cost_type max_cost = 
            std::numeric_limits<cost_type>::max();

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
    };
}