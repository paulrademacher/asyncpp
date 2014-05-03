#pragma once

#ifndef __ASYNC_SERIES_HPP__
#define __ASYNC_SERIES_HPP__

#include <atomic>
#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "debug.hpp"

namespace async {

template<typename T>
using SeriesCallback = std::function<void(ErrorCode error, T result)>;

template<typename T>
void noop_series_callback(ErrorCode e, T result) {};

template<typename T>
using SeriesCompletionCallback = std::function<void(ErrorCode, std::vector<T>&)>;

template<typename T>
void noop_series_final_callback(ErrorCode e, std::vector<T> p) {};

template <typename T>
using Task = std::function<void(SeriesCallback<T>&)>;

template<typename T>
using TaskVector = std::vector<Task<T>>;

// This value can be asserted to equal zero if there's no pending series callbacks.
// Otherwise, if it's non-zero after all series callbacks have executed, we have a memory
// leak.
std::atomic<int> debug_series_state_count(0);

// `tasks` and `final_callback` are passed by reference.  It is the responsibility of the
// caller to ensure that their lifetime exceeds the lifetime of the series call.
template<typename T>
void series(std::vector<Task<T>> &tasks,
    const SeriesCompletionCallback<T> &final_callback=noop_series_final_callback<T>) {

  struct SeriesState {
    SeriesCallback<T> callback;
    std::function<void()> invoke_until_async;
    std::vector<Task<T>> *tasks;
    typename TaskVector<T>::iterator iter;
    bool is_inside_task;
    bool callback_called;
    std::vector<T> results;
    const SeriesCompletionCallback<T> *final_callback;

    SeriesState() {
      debug_series_state_count++;
      printf("> SeriesState\n");
    }

    ~SeriesState() {
      debug_series_state_count--;
      printf("< SeriesState\n\n");
    }
  };

  auto state = std::make_shared<SeriesState>();
  state->tasks = &tasks;
  state->final_callback = &final_callback;
  state->iter = tasks.begin();

  // If task list is empty, invoke the final callback immediately with a success code and
  // an empty results list.
  if (state->tasks->size() == 0) {
    final_callback(OK, state->results);
    return;
  }

  // Capture tasks by reference to not copy the vector.  `iter` is bound to the original
  // vector.
  state->callback = [state](ErrorCode error, T result) mutable {
    // DebugScope d("callback");
    assert(state);

    state->callback_called = true;
    state->results.push_back(result);

    if (state->iter == state->tasks->end()) {
      // We're done.  No more tasks, so no more callbacks.
      printf("finished callbacks\n");
      printf("cb state count: %ld\n", state.use_count());

      (*state->final_callback)(error, state->results);

      state.reset();
    } else if (!state->is_inside_task) {
      state->invoke_until_async();
    }
  };

  // Capture tasks by reference to not copy the vector.  `iter` is bound to the original
  // vector.
  state->invoke_until_async = [state]() mutable {
    assert(state);

    // DebugScope d("invoke_until_async");
    while (state->iter != state->tasks->end()) {
      state->is_inside_task = true;
      state->callback_called = false;
      auto task = *(state->iter);
      ++(state->iter);

      task(state->callback);

      state->is_inside_task = false;

      if (!state->callback_called) {
        // The callback wasn't called, which means it was deferred.  Stop iterating.
        // The future invocation of the callback will continue the iteration.
        break;
      }
    }

    if (state->iter == state->tasks->end()) {
      // No more tasks to process.  Release the shared pointer to `state`.
      printf("finished invoke\n");
      printf("invoke state count: %ld\n", state.use_count());
      state.reset();
    }
  };

  state->invoke_until_async();

  state.reset();
}

}

#endif
