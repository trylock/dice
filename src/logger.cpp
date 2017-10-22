#include "logger.hpp"

dice::logger::logger() : output_(&std::cerr) {}

dice::logger::logger(std::ostream* output) : output_(output)
{
    assert(output_ != nullptr);
}

void dice::logger::error(int line, int col, const std::string& message)
{
    *output_ << "\e[1m" // bold text
        << "[line: " << line << "]" 
        << "[col: " << col << "] "
        << "\e[0m"; // reset formatting
    *output_ << "\e[1;31m" // bold read text
        << "error:"
        << "\e[0m "; // reset formatting
    *output_ << message << std::endl;
    has_error_ = true;
}

bool dice::logger::empty() const 
{
    return !has_error_;
}
