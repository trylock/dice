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
    /** @brief This class prints all errors in given output stream.
     *
     * Example usage:
     * @code
     * logger log{ &std::cin };
     * assert(log.empty());
     * log.error(0, 14, "Critical error");
     * assert(!log.empty());
     * @endcode
     */
    class logger
    {
    public:
        /** @brief Use std::cerr as the output of the logger. */
        logger();

        /** @brief Use given output stream as the output of the logger.
         *
         * @param output pointer to an output stream
         * @param print_just_message if true, the logger will only print 
         *                           unformated error message (no error 
         *                           location or message formatting)
         */
        explicit logger(std::ostream* output, bool print_just_message = false);

        /** @brief Add a new error log
         *
         * @param line of the input at which the error occured
         * @param column in which the error occured
         * 
         * @param message of the error
         */
        void error(int line, int column, const std::string& message);

        /** @brief Check whether there were some errors logged.
         *
         * @return true IFF there were no errors
         */
        bool empty() const;
    private:
        std::ostream* output_;
        bool has_error_ = false;
		bool print_just_message_ = false;
    };
}

#endif // DICE_ERRORS_HPP_