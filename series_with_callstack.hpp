// THIS IS UNUSED.  FOR REFERENCE ONLY.

#pragma once

#ifndef __ASYNC_SERIES_WITH_CALLSTACK_HPP__
#define __ASYNC_SERIES_WITH_CALLSTACK_HPP__

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace async {

template<typename T>
using SeriesCallback = std::function<void(ErrorCode error, T result)>;

template<typename T>
void noop_series_final_callback(ErrorCode e, std::vector<T> p) {};

template <typename T>
using Task = std::function<void(SeriesCallback<T>&)>;

template<typename T>
using TaskVector = std::vector<Task<T>>;

// `tasks` and `final_callback` are passed by reference.  It is the responsibility of the
// caller to ensure that their lifetime exceeds the lifetime of the series call.
template<typename T>
void series_with_callstack(std::vector<Task<T>> &tasks,
    const std::function<void(ErrorCode, std::vector<T>&)> &final_callback=noop_series_final_callback<T>) {

  auto results = std::make_shared<std::vector<T>>();

  // If task list is empty, invoke the final callback immediately with a success code and
  // an empty results list.  Doing so now simplifies the iteration below.
  if (tasks.size() == 0) {
    if (final_callback) {
      final_callback(OK, results);
    }
    return;
  }

  using Iterator = typename TaskVector<T>::iterator;

  Iterator task_iter = tasks.begin();
  SeriesCallback<T> callback = [&tasks, task_iter, &final_callback, results, &callback](ErrorCode error, T result) mutable {
    //    printf("err=%d   iter: %x %x   - &iter=%x  &cb=%x\n", error, task_iter, tasks.end(), &task_iter, &callback);
    results->push_back(result);
    if (error == OK) {
      ++task_iter;
      if (task_iter == tasks.end()) {
        final_callback(error, results);
      } else {
        // `callback` is being passed by reference.  If it were by value, then we'd be
        // creating a copy of callback and all the captured variables, and task_iter would
        // be effectively reset.
        (*task_iter)(callback);
      }
    } else {
      final_callback(error, results);
    }
  };

  (*task_iter)(callback);
}

}

#endif
