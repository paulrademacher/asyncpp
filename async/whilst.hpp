#pragma once

#ifndef __ASYNC_WHILST_HPP__
#define __ASYNC_WHILST_HPP__

#include "forever_iterator.hpp"

namespace async {

void noop_whilst_final_callback(ErrorCode) {};

/**
   Run tasks, one at a time.  This simply calls parallel_limit() with a limit of 1.
 */

// `tasks` and `final_callback` are passed by reference.  It is the responsibility of the
// caller to ensure that their lifetime exceeds the lifetime of the whilst call.
void whilst(std::function<bool()> test, std::function<void(std::function<void(ErrorCode)>)> func,
    const std::function<void(ErrorCode)> &final_callback=noop_whilst_final_callback) {

  async::ForeverIterator start;
  async::ForeverIterator stop;

  auto wrapped_callback = [&func, &test](int item, int index, bool is_last_time,
      std::function<void(bool, ErrorCode)> callback_done) {

    auto task_callback = [callback_done](ErrorCode error) {
      callback_done(error == OK, error);
    };

    if (test()) {
      func(task_callback);
    } else {
      // Stop iterating.
      callback_done(false, OK);
    }
  };

  sequencer<int, decltype(start)>(start, stop, 1, wrapped_callback, final_callback);
}

}

#endif
