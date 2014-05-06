#pragma once

#ifndef __ASYNC_ASYNC_HPP__
#define __ASYNC_ASYNC_HPP__

#include <functional>
#include <vector>

namespace async {

typedef enum {
  OK = 0,
  FAIL = -1
} ErrorCode;

template<typename T>
using TaskCallback = std::function<void(ErrorCode error, T result)>;

template<typename T>
void noop_task_callback(ErrorCode e, T result) {};

template<typename T>
using TaskCompletionCallback = std::function<void(ErrorCode, std::vector<T>&)>;

template<typename T>
void noop_task_final_callback(ErrorCode e, std::vector<T> p) {};

}

#include "lib/map.hpp"
#include "lib/series.hpp"

#endif
