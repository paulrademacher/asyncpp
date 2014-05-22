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
void series_full(std::vector<SeriesTask<T>> &tasks,
    const TaskCompletionCallback<T> &final_callback=noop_task_final_callback<T>) {

  struct State {
    TaskCallback<T> callback;
    std::function<void()> invoke_until_deferred_callback;
    std::vector<SeriesTask<T>> *tasks;
    typename SeriesTaskVector<T>::iterator iter;
    bool is_inside_task;
    bool callback_called;
    std::vector<T> results;
    const TaskCompletionCallback<T> *final_callback;

    State() {
      priv::series_state_count++;
    }

    ~State() {
      priv::series_state_count--;
    }
  };

  auto state = std::make_shared<State>();
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

      // Empty the callback inside the State object.  This releases the shared_ptr
      // inside the callback which points back to the State itself.  When the
      // original instance of this callback, and all copies, and the function
      // `invoke_until_deferred_callback` are all released (go out of scope or are explicity deleted),
      // then there will be no more shared_ptrs to State, and State will be deleted.

      // Note that it is not sufficient here to reset the `state` shared_ptr.  Consider
      // this example: Let's say a SeriesTask defers the call to `callback`, and stores a
      // *copy* of that callback, to be called later.  The original callback is inside
      // State, but the copy is not.  When the callback-copy is invoked at some
      // point in the future, it will run through its logic, eventually completing the
      // series and ending up at this point.  Then assume it reset its state shared_ptr.
      // That is good to free up that pointer... however, the original instance of the
      // callback lambda still holds a `series` shared_ptr.  Because that original lambda
      // is inside State, is won't go out of scope until the State does.  But
      // the lambda holds a pointer to the State, so State won't ever go out
      // of scope --> Memory leak.  By removing the original callback from State, we
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

      // Clear the callback inside the State, to break the ownership cycle and allow
      // State to go out of scope once callbacks complete.  See comments inside
      // `callback` function for details.
      state->invoke_until_deferred_callback = [](){};
    }
  };

  state->invoke_until_deferred_callback();
}

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
  run_sequence<SeriesTask<T>, decltype(tasks_begin), int>
      (tasks_begin, tasks_end, 1, data, callback, wrapped_final_callback);
}

}

#endif
