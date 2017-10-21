#ifndef DICE_ERRORS_HPP_
#define DICE_ERRORS_HPP_

#include <string>
#include <vector>
#include <ostream>
#include <iostream>

namespace dice
{
    struct error_log 
    {
        int line;
        int col;
        std::string message;
    };

    class errors
    {
    public:
        using const_iterator = std::vector<error_log>::const_iterator;

        /** Add a new error to the collection
         * @param line of the input at which the error occured
         * @param column in which the error occured
         * @param error message
         */
        void add(int line, int col, const std::string& message);

        // Iterator of the first error log
        const_iterator begin() const;
        // Iterator of the last error log
        const_iterator end() const;
        // true <=> there are no errors
        bool empty() const;

    private:
        std::vector<error_log> errors_;
    };

    /** Print errors to the output stream.
     * @param output stream
     * @param errors object
     * @return pass the output stream
     */
    std::ostream& operator<<(std::ostream& output, const errors& errs);
}

#endif // DICE_ERRORS_HPP_