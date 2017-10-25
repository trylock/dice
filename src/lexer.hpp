#ifndef DICE_LEXER_HPP_
#define DICE_LEXER_HPP_

#include <string>
#include <sstream>
#include <cctype>
#include <istream>

#include "logger.hpp"
#include "symbols.hpp"

namespace dice 
{
    // location in the input stream
    struct lexer_location
    {
        int line = 0;
        int col = 0;
    };

    class lexer 
    {
    public:
        lexer(std::istream* input, logger* log);

        // disallow copy so that 2 lexers don't read from the same input
        lexer(const lexer&) = delete;
        void operator=(const lexer&) = delete;

        // allow move
        lexer(lexer&&) = default;
        lexer& operator=(lexer&&) = default;

        /** Read token at current location.
         * Decode a sequance of characters as a token and 
         * advance in the input stream.
         * @return token at current position
         */
        symbol read_token();

        // Return current location of the lexer in the input stream
        inline const lexer_location& location() const 
        {
            return location_; 
        }

    private:
        std::istream* input_;
        logger* log_;
        lexer_location location_;

        // skip sequence of whitespace at current location
        void skip_space();

        // get character at current location
        int get_char();

        // report a lexer error
        void error(const std::string& message);
    };
}

#endif // DICE_LEXER_HPP_