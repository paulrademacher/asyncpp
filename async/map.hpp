#pragma once

#ifndef __ASYNC_MAP_HPP__
#define __ASYNC_MAP_HPP__

#include "sequencer.hpp"

namespace async {

// TODO: Make versions that take reference and also another with by-value callbacks, to
// support lambda decls inline in function calls.

// `objects`, `func`, and `final_callback` are passed by reference.  It is the
// responsibility of the caller to ensure that their lifetime exceeds the lifetime of the
// series call.
template<typename T>
void map(std::vector<T> objects,
    std::function<void(T, TaskCallback<T>)> func,
    const TaskCompletionCallback<T> &final_callback=noop_task_final_callback<T>,
    unsigned int task_limit=0) {

  std::vector<T>* results = new std::vector<T>(objects.size());

  auto objects_begin = begin(objects);
  auto objects_end = end(objects);

  auto callback = [results, func](T object, int index, bool is_last_time,
      std::function<void(bool, ErrorCode)> callback_done) {

    TaskCallback<T> task_callback = [callback_done, results, index](ErrorCode error, T result) {
      (*results)[index] = result;
      callback_done(error == OK, error);
    };

    func(object, task_callback);
  };

  auto wrapped_final_callback = [results, final_callback](ErrorCode error) {
    final_callback(error, *results);
    delete results;
  };

  sequencer<T, decltype(objects_begin)>
      (objects_begin, objects_end, task_limit, callback, wrapped_final_callback);
}

}

#endif
