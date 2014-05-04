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
using SeriesTask = std::function<void(SeriesCallback<T>&)>;

template<typename T>
using SeriesTaskVector = std::vector<SeriesTask<T>>;

// This value can be asserted to equal zero if there's no pending series callbacks.
// Otherwise, if it's non-zero after all series callbacks have executed, we have a memory
// leak.
namespace priv {
std::atomic<int> series_state_count(0);
}

int get_state_count() {
  return priv::series_state_count;
}

// `tasks` and `final_callback` are passed by reference.  It is the responsibility of the
// caller to ensure that their lifetime exceeds the lifetime of the series call.
template<typename T>
void series(std::vector<SeriesTask<T>> &tasks,
    const SeriesCompletionCallback<T> &final_callback=noop_series_final_callback<T>) {

  struct SeriesState {
    SeriesCallback<T> callback;
    std::function<void()> invoke_until_deferred_callback;
    std::vector<SeriesTask<T>> *tasks;
    typename SeriesTaskVector<T>::iterator iter;
    bool is_inside_task;
    bool callback_called;
    std::vector<T> results;
    const SeriesCompletionCallback<T> *final_callback;

    SeriesState() {
      priv::series_state_count++;
    }

    ~SeriesState() {
      priv::series_state_count--;
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

  state->callback = [state](ErrorCode error, T result) mutable {
    assert(state);

    state->callback_called = true;
    state->results.push_back(result);

    if (error != OK) {
      // Stop iterating.
      state->iter = state->tasks->end();
    }

    if (state->iter == state->tasks->end()) {
      // We're done.  No more tasks, so no more callbacks.
      (*state->final_callback)(error, state->results);

      // Empty the callback inside the SeriesState object.  This releases the shared_ptr
      // inside the callback which points back to the SeriesState itself.  When the
      // original instance of this callback, and all copies, and the function
      // `invoke_until_deferred_callback` are all released (go out of scope or are explicity deleted),
      // then there will be no more shared_ptrs to SeriesState, and SeriesState will be deleted.

      // Note that it is not sufficient here to reset the `state` shared_ptr.  Consider
      // this example: Let's day a SeriesTask defers the call to `callback`, and stores a
      // *copy* of that callback, to be called later.  The original callback is inside
      // SeriesState, but the copy is not.  When the callback-copy is invoked at some
      // point in the future, it will run through its logic, eventually completing the
      // series and ending up at this point.  Then assume it reset its state shared_ptr.
      // That is good to free up that pointer... however, the original instance of the
      // callback lambda still holds a `series` shared_ptr.  Because that original lambda
      // is inside SeriesState, is won't go out of scope until the SeriesState does.  But
      // the lambda holds a pointer to the SeriesState, so SeriesState won't ever go out
      // of scope --> Memory leak.  By removing the original callback from SeriesState, we
      // break the cycle.
      state->callback = [](ErrorCode e, T r){};
    } else if (!state->is_inside_task) {
      state->invoke_until_deferred_callback();
    }
  };

  state->invoke_until_deferred_callback = [state]() mutable {
    assert(state);

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
      // No more tasks to process.

      // Clear the callback inside the SeriesState, to break the ownership cycle and allow
      // SeriesState to go out of scope once callbacks complete.  See comments inside
      // `callback` function for details.
      state->invoke_until_deferred_callback = [](){};
    }
  };

  state->invoke_until_deferred_callback();
}

}

#endif
