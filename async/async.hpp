#pragma once

#ifndef __ASYNC_ASYNC_HPP__
#define __ASYNC_ASYNC_HPP__

#include <cassert>
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
void noop_task_final_callback(ErrorCode e, std::vector<T>& p) {};

template <typename T>
using Task = std::function<void(TaskCallback<T>&)>;

template<typename T>
using TaskVector = std::vector<Task<T>>;

using BoolCallback = std::function<void(bool)>;

}

#include "filter.hpp"
#include "map.hpp"
#include "parallel.hpp"
#include "series.hpp"
#include "sequencer.hpp"
#include "whilst.hpp"

#endif
