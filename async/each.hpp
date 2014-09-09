#pragma once

#ifndef __ASYNC_EACH_HPP__
#define __ASYNC_EACH_HPP__

#include "sequencer.hpp"

namespace async {

// `data`, `func`, and `final_callback` are passed by reference.  It is the
// responsibility of the caller to ensure that their lifetime exceeds the lifetime of the
// series call.
template<typename T>
void each(std::vector<T> &data,
    std::function<void(T, ErrorCodeCallback)> func,
    const ErrorCodeCallback &final_callback=noop_error_code_final_callback,
    unsigned int task_limit=0) {

  auto callback = [func](T object, int index, bool is_last_time,
      std::function<void(bool, ErrorCode)> callback_done) {

    auto task_callback = [callback_done, index](ErrorCode error) {
      callback_done(error == OK, error);
    };

    func(object, task_callback);
  };

  auto wrapped_final_callback = [final_callback](ErrorCode error) {
    final_callback(error);
  };

  sequencer<T>
      (data.begin(), data.end(), task_limit, callback, wrapped_final_callback);
}

}

#endif
