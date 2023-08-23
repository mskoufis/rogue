#pragma once

#include <boost/python.hpp>

namespace rogue { 
class PythonError : public std::exception { 
 public: 
    const char* what() const noexcept override; 
};
} // namespace rogue
