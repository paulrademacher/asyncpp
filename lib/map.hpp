#pragma once

#ifndef __ASYNC_MAP_HPP__
#define __ASYNC_MAP_HPP__

namespace async {

// This value can be asserted to equal zero if there's no pending callbacks.  Otherwise,
// if it's non-zero after all callbacks have executed, we have a memory leak.
namespace priv {
int map_state_count = 0;
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
    std::function<void()> spawn_one;
    std::vector<T> results;
    std::shared_ptr<State> keep_alive;
    unsigned int results_count;
    unsigned int task_limit;
    unsigned int task_counter;
    bool reached_limit;
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
  state->results_count = 0;
  state->task_limit = task_limit;
  state->task_counter = 0;
  state->reached_limit = false;

  state->spawn_one = [state, &func, &objects]() mutable {
    int index = state->task_counter;
    auto object = objects[index];

    if (state->task_counter == state->task_limit - 1) {
      state->reached_limit = true;
    }

    state->task_counter++;

    func(object, [index, state](ErrorCode error, T result) {
          if (state->had_error) {
            // There was an error code earlier, so work has finished.
            return;
          }

          state->results[index] = result;
          state->results_count++;
          if (error != OK) {
            state->had_error = true;
          }

          if (state->had_error || state->results_count == state->results.size()) {
            // All results have been collected.  We're done.

            (*state->final_callback)(error, state->results);
            state->spawn_one = [](){};  // Break the circular dependency, allowing 'state'
                                        // to release.
            state->keep_alive.reset();
          } else if (state->reached_limit) {
            // We've spawned as many tasks as our limit in the initial loop.
            // Now, as each task completes, spawn one more if there are still
            // tasks on the queue.
            if (state->task_counter < state->results.size()) {
              state->spawn_one();
            }
          }
        });
  };

  while (state->task_counter < task_limit) {
    state->spawn_one();
  }
}

}

#endif
