#pragma once

#ifndef ASYNC_PARALLEL_HPP
#define ASYNC_PARALLEL_HPP

#include "sequencer.hpp"

namespace async {

/**
   Run a sequence of tasks, in parallel, up to 'limit' at a time.  Each task is a
   std::function, which must accept a callback function which itself accepts an error code
   and a result value.  As each task is invoked, it should perform some work, then invoke
   the callback when it finishes.  If the callback is given some error_code that is not
   async::OK, then iteration stops and no more tasks are invoked.  When all tasks complete
   successfully or some task passes an error to its callback, then the `final_callback`
   will be called with the last error code and a vector of all return values.

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
// caller to ensure that their lifetime exceeds the lifetime of the parallel call.
template<typename T>
void parallel_limit(std::vector<Task<T>> &tasks,
    unsigned int limit,
    const TaskCompletionCallback<T> &final_callback=noop_task_final_callback<T>) {

  std::vector<T>* results = new std::vector<T>();

  auto callback = [results](Task<T> task, int index, bool is_last_time,
      std::function<void(bool, ErrorCode)> callback_done) {

    TaskCallback<T> task_callback = [callback_done, results, index](ErrorCode error, T result) {
      results->push_back(result);
      callback_done(error == OK, error);
    };
    task(task_callback);
  };

  auto wrapped_final_callback = [results, final_callback](ErrorCode error) {
    final_callback(error, *results);
    delete results;
  };

  sequencer<Task<T>>
      (tasks.begin(), tasks.end(), limit, callback, wrapped_final_callback);
}

/**
   Runs tasks in parallel, with no limit on number of concurrent tasks.
*/

template<typename T>
void parallel(std::vector<Task<T>> &tasks,
    const TaskCompletionCallback<T> &final_callback=noop_task_final_callback<T>) {

  parallel_limit(tasks, 0, final_callback);
}

}

#endif
