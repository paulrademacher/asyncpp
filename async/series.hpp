#pragma once

#ifndef __ASYNC_SERIES_HPP__
#define __ASYNC_SERIES_HPP__

#include <cassert>
#include <iostream>
#include <string>

#include "debug.hpp"

#include "sequencer.hpp"

namespace async {

template <typename T>
using SeriesTask = std::function<void(TaskCallback<T>&)>;

template<typename T>
using SeriesTaskVector = std::vector<SeriesTask<T>>;

// This value can be asserted to equal zero if there's no pending series callbacks.
// Otherwise, if it's non-zero after all series callbacks have executed, we have a memory
// leak.
namespace priv {
int series_state_count;
}

int get_series_state_count() {
  return priv::series_state_count;
}

/**
   `series` runs a sequence of tasks, one after the other.  Each task is a std::function,
   which must accept a callback function which itself accepts an error code and a result
   value.  As each task is invoked, it should perform some work, then invoke the callback
   when it finishes.  If the callback is given some error_code that is not async::OK, then
   iteration stops and no more tasks are invoked.  When all tasks complete successfully or
   some task passes an error to its callback, then the `final_callback` will be called
   with the last error code and a vector of all return values.

   It is the responsibility of the caller that the `tasks` vector and `final_callback`
   passed to this function are not destroyed until `final_callback` is invoked.

   Notes:

   1. When a task is invoked, it may invoke its callback function immediately, or it may
     defer the callback.  For example, if Boost ASIO library is used to make an HTTP
     fetch or a deadline timer is started, the callback won't be invoked till some time
     in the future.  `series` handles that case correctly, and works if the callback is
     called immediately or deferred.
  2. `series` minimizes how much stack it uses.  Except for deferred callbacks, we
     don't invoke the next task from the previous task's callback -- otherwise the
     stack would grow on every task invocation.
 */

// `tasks` and `final_callback` are passed by reference.  It is the responsibility of the
// caller to ensure that their lifetime exceeds the lifetime of the series call.
template<typename T>
void series(std::vector<SeriesTask<T>> &tasks,
    const TaskCompletionCallback<T> &final_callback=noop_task_final_callback<T>) {

  std::vector<T>* results = new std::vector<T>();

  auto tasks_begin = begin(tasks);
  auto tasks_end = end(tasks);

  auto callback = [results](SeriesTask<T> task, int index, bool is_last_time, int& data_dummy,
      std::function<void(bool, ErrorCode)> callback_done) {

    TaskCallback<T> task_callback = [callback_done, results, index](ErrorCode error, T result) {
      results->push_back(result);
      callback_done(error == OK, error);
    };
    task(task_callback);
  };

  auto wrapped_final_callback = [results, final_callback](ErrorCode error, int& data_dummy) {
    final_callback(error, *results);
    delete results;
  };

  int data = 0;
  sequencer<SeriesTask<T>, decltype(tasks_begin), int>
      (tasks_begin, tasks_end, 1, data, callback, wrapped_final_callback);
}

}

#endif
