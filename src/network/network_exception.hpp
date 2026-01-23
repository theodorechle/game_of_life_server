#ifndef NETWORK_EXCEPTION_HPP
#define NETWORK_EXCEPTION_HPP

#include <exception>
#include <string>

class NetworkException : public std::exception {
    std::string _message;

public:
    NetworkException(const std::string &message) : _message{message} {}
    const char *what() const noexcept override { return _message.c_str(); }
};

#endif // NETWORK_EXCEPTION_HPP
