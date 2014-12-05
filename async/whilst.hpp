#pragma once

#ifndef ASYNC_WHILST_HPP
#define ASYNC_WHILST_HPP

#include "forever_iterator.hpp"
#include "sequencer.hpp"

namespace async {

void noop_whilst_final_callback(ErrorCode) {};

/**
   Executes tasks while `test` returns `true`, and while `func` passes `OK` to its
   callback.  Equivalent to `while` control flow.
 */
void whilst(const std::function<bool()> &test,
    const std::function<void(ErrorCodeCallback)> &func,
    const ErrorCodeCallback &final_callback=noop_whilst_final_callback) {

  auto wrapped_callback = [func, test](int item, int index, bool is_last_time,
      std::function<void(bool, ErrorCode)> callback_done) {
    auto task_callback = [callback_done](ErrorCode error) {
      callback_done(error == OK, error);
    };

    if (test()) {
      func(task_callback);
    } else {
      // Forever_Iter_Stop iterating.
      callback_done(false, OK);
    }
  };

  sequencer<int, ForeverIterator>
      (ForeverIteratorInstance, ForeverIteratorInstance, 1, wrapped_callback, final_callback);
}

/**
   Executes tasks while `test` returns `true`, and while `func` passes `OK` to its
   callback.  Equivalent to `do..while` control flow.
 */
void doWhilst(const std::function<void(std::function<void(ErrorCode)>)> &func,
    const std::function<bool()> &test,
    const std::function<void(ErrorCode)> &final_callback=noop_whilst_final_callback) {

  auto wrapped_callback = [func, &test](int item, int index, bool is_last_time,
      std::function<void(bool, ErrorCode)> callback_done) {
    auto task_callback = [callback_done, test](ErrorCode error) {
      callback_done(error == OK && test(), error);
    };

    func(task_callback);
  };

  sequencer<int, ForeverIterator>
      (ForeverIteratorInstance, ForeverIteratorInstance, 1, wrapped_callback, final_callback);
}


/**
   Perform task until `test` returns true, or until `func` does not pass `OK` to its
   callback.  Uses `while` control flow.  This is inverse of `whilst`.
 */
void until(const std::function<bool()> &test,
    const std::function<void(std::function<void(ErrorCode)>)> &func,
    const std::function<void(ErrorCode)> &final_callback=noop_whilst_final_callback) {
  whilst([test]() { return !test(); }, func, final_callback);
}


/**
   Perform task until test returns true, or until `func` does not pass `OK` to its
   callback.  Uses `do..while` control flow.  This is inverse of `doWhilst`.
 */
void doUntil(const std::function<void(std::function<void(ErrorCode)>)> &func,
    const std::function<bool()> &test,
    const std::function<void(ErrorCode)> &final_callback=noop_whilst_final_callback) {
  doWhilst(func, [test]() { return !test(); }, final_callback);
}


/**
   Perform task until `func` does not pass `OK` to its callback.
 */
void forever(const std::function<void(std::function<void(ErrorCode)>)> &func,
    const std::function<void(ErrorCode)> &final_callback=noop_whilst_final_callback) {
  whilst([]() { return true; }, func, final_callback);
}

/**
   Perform task a fixed number of times, or until `func` does not pass `OK` to its callback.
 */
void ntimes(int times, const std::function<void(std::function<void(ErrorCode)>)> &func,
    const std::function<void(ErrorCode)> &final_callback=noop_whilst_final_callback) {
  int count = 0;
  whilst([count, times]() mutable {
        bool keep_going = count < times;
        count++;
        return keep_going;
      },
      func, final_callback);
}

}

#endif
