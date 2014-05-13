#pragma once

#ifndef __ASYNC_MAP_HPP__
#define __ASYNC_MAP_HPP__

#include <atomic>

namespace async {

// This value can be asserted to equal zero if there's no pending callbacks.  Otherwise,
// if it's non-zero after all callbacks have executed, we have a memory leak.
namespace priv {
std::atomic<int> map_state_count(0);
}

int get_map_state_count() {
  return priv::map_state_count;
}

template<typename T>
void map(std::vector<T> objects,
    std::function<void(T, TaskCallback<T>)> func,
    const TaskCompletionCallback<T> &final_callback=noop_task_final_callback<T>,
    unsigned int task_limit=0) {

  if (task_limit == 0 || task_limit > objects.size()) {
    task_limit = objects.size();
  }

  struct State {
    std::vector<T> results;
    std::shared_ptr<State> keep_alive;
    std::atomic<int> results_count;
    unsigned int task_limit;
    const TaskCompletionCallback<T> *final_callback;
    bool had_error = false;

    State() {
      priv::map_state_count++;
      printf("> STATE\n");
    }

    ~State() {
      priv::map_state_count--;
      printf("< STATE\n");
    }
  };

  auto state = std::make_shared<State>();
  state->results = std::vector<T>(objects.size());
  state->keep_alive = state;
  state->final_callback = &final_callback;
  state->results_count.store(0);
  state->task_limit = task_limit;

  for (int i = 0; i < objects.size(); i++) {
    auto object = objects[i];
    func(object, [i, state](ErrorCode error, T result) {
          if (state->had_error) {
            // There was an error code earlier, so work has finished.
            return;
          }

          state->results[i] = result;

          state->results_count++;

          if (error != OK) {
            state->had_error = true;
          }

          if (state->had_error || state->results_count == state->results.size()) {
            // All results have been collected.

            (*state->final_callback)(error, state->results);
            state->keep_alive.reset();
          }
        });
  }
}

}

#endif
