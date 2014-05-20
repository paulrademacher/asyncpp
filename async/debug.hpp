#pragma once

#ifndef __ASYNC_DEBUG_HPP__
#define __ASYNC_DEBUG_HPP__

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


