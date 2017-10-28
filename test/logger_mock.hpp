#ifndef LOGGER_MOCK_HPP_
#define LOGGER_MOCK_HPP_

#include <vector>
#include <string>

struct log_entry 
{
    int line;
    int col;
    std::string message;
};

class logger_mock 
{
public:
    void error(int line, int col, const std::string& message)
    {
        errors_.push_back(log_entry{
            line,
            col,
            message
        });
    }

    const std::vector<log_entry>& errors() const { return errors_;  }

private:
    std::vector<log_entry> errors_;
};

#endif // LOGGER_MOCK_HPP_