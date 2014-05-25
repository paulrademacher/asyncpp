#pragma once

#ifndef __ASYNC_SERIES_HPP__
#define __ASYNC_SERIES_HPP__

#include "parallel.hpp"

namespace async {

/**
   Run tasks, one at a time.  This simply calls parallel_limit() with a limit of 1.
 */

// `tasks` and `final_callback` are passed by reference.  It is the responsibility of the
// caller to ensure that their lifetime exceeds the lifetime of the series call.
template<typename T>
void series(std::vector<Task<T>> &tasks,
    const TaskCompletionCallback<T> &final_callback=noop_task_final_callback<T>) {

  parallel_limit(tasks, 1, final_callback);
}

}

#endif
