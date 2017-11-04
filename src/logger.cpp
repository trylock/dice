#include "logger.hpp"

dice::logger::logger() : output_(&std::cerr) {}

dice::logger::logger(std::ostream* output, bool print_just_message) : 
	output_(output), 
	print_just_message_(print_just_message)
{
    assert(output_ != nullptr);
}

void dice::logger::error(int line, int col, const std::string& message)
{
	if (!print_just_message_)
	{
		*output_ << termcolor::bold
			<< "[line: " << line << "]"
			<< "[col: " << col << "] "
			<< termcolor::reset;

		*output_ << termcolor::bold << termcolor::red
			<< "error: "
			<< termcolor::reset;
	}
    *output_ << message << std::endl;
    has_error_ = true;
}

bool dice::logger::empty() const 
{
    return !has_error_;
}
