#pragma once

#ifndef ASYNC_DEBUG_HPP
#define ASYNC_DEBUG_HPP

#include <iostream>

namespace async {

class DebugScope {
public:
  DebugScope(std::string msg) : msg_(msg) {
    std::cout << "> " << msg_ << std::endl;
  };

  ~DebugScope() {
    std::cout << "< " << msg_ << std::endl;
  };

private:
  std::string msg_;
};

}

#endif


