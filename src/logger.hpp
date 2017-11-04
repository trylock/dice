#ifndef DICE_ERRORS_HPP_
#define DICE_ERRORS_HPP_

#include <string>
#include <vector>
#include <ostream>
#include <iostream>
#include <cassert>

#include <termcolor.hpp>

namespace dice
{
    class logger
    {
    public:
        // Use std::cerr as the output of the logger
        logger();

        // Use given output stream as the output of the logger
        explicit logger(std::ostream* output, bool print_just_message = false);

        /** Add a new error log
         * @param line of the input at which the error occured
         * @param column in which the error occured
         * @param error message
         */
        void error(int line, int col, const std::string& message);

        // true <=> there are no errors
        bool empty() const;
    private:
        std::ostream* output_;
        bool has_error_ = false;
		bool print_just_message_ = false;
    };
}

#endif // DICE_ERRORS_HPP_