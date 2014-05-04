#pragma once

#ifndef __ASYNC_ASYNC_HPP__
#define __ASYNC_ASYNC_HPP__

namespace async {

typedef enum {
  OK = 0,
  FAIL = -1,
  DEBUG_COMMAND = -2
} ErrorCode;

}

#include "series.hpp"

#endif
