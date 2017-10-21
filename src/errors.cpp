#include "errors.hpp"

void dice::errors::add(int line, int col, const std::string& message)
{
    errors_.push_back(error_log{
        line,
        col,
        message
    });
}

dice::errors::const_iterator dice::errors::begin() const
{
    return errors_.cbegin();
}

dice::errors::const_iterator dice::errors::end() const 
{
    return errors_.cend();
}

bool dice::errors::empty() const 
{
    return errors_.empty();
}

std::ostream& dice::operator<<(std::ostream& output, const errors& errs)
{
    for (auto&& error : errs)
    {
        output << "\e[1m" // bold text
                << "[line: " << error.line << "]" 
                << "[col: " << error.col << "] "
                << "\e[0m"; // reset formatting
        output << "\e[1;31m" // bold read text
                << "error:"
                << "\e[0m "; // reset formatting
        output << error.message << std::endl;
    }
    return output;
}