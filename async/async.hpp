#pragma once

#ifndef ASYNC_ASYNC_HPP
#define ASYNC_ASYNC_HPP

#include <cassert>
#include <functional>
#include <vector>

#include "debug.hpp"

namespace async {

typedef enum {
  OK = 0,
  FAIL = -1,
  STOP = -2
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

using ErrorCodeCallback = std::function<void(ErrorCode)>;
inline void noop_error_code_final_callback(ErrorCode e) {};

}

#include "each.hpp"
#include "filter.hpp"
#include "map.hpp"
#include "parallel.hpp"
#include "series.hpp"
#include "sequencer.hpp"
#include "whilst.hpp"

#endif
