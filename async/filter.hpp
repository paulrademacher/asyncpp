#pragma once

#ifndef __ASYNC_FILTER_HPP__
#define __ASYNC_FILTER_HPP__

#include "forever_iterator.hpp"
#include "sequencer.hpp"

namespace async {

template<typename T>
void noop_filter_final_callback(std::vector<T> &results) {};

template <typename T>
void filter(std::vector<T> &data,
    const std::function<void(T, BoolCallback)> &test,
    const std::function<void(std::vector<T> &results)> &final_callback=noop_filter_final_callback,
    bool invert=false) {

  std::vector<bool>* keep = new std::vector<bool>(data.size());

  auto wrapped_callback = [invert, keep, test](T item, int index, bool is_last_time,
      std::function<void(bool, ErrorCode)> callback_done) {
    BoolCallback task_callback = [callback_done, &index, invert, &item, keep](bool truth) {
      if (invert) {
        (*keep)[index] = !truth;
      } else {
        (*keep)[index] = truth;
      }

      callback_done(true, OK);
    };

    test(item, task_callback);
  };

  auto wrapped_final_callback = [&data, final_callback, keep](ErrorCode error) {
    std::vector<T> results;

    for (int i = 0; i < keep->size(); i++) {
      if ((*keep)[i]) {
        results.push_back(data[i]);
      }
    }

    final_callback(results);

    delete keep;
  };

  sequencer<T>
      (data.begin(), data.end(), 0, wrapped_callback, wrapped_final_callback);
}

template <typename T>
void reject(std::vector<T> &data,
    const std::function<void(T, BoolCallback)> &test,
    const std::function<void(std::vector<T> &results)> &final_callback=noop_filter_final_callback) {

  filter(data, test, final_callback, true);
}

}

#endif
