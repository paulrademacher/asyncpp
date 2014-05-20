#pragma once

#ifndef __ASYNC_SEQUENCER_HPP__
#define __ASYNC_SEQUENCER_HPP__

namespace async {

// This value can be asserted to equal zero if there's no pending callbacks.  Otherwise,
// if it's non-zero after all callbacks have executed, we have a memory leak.
namespace priv {
int sequencer_state_count = 0;
}

int get_sequencer_state_count() {
  return priv::sequencer_state_count;
}

using VoidFunction = std::function<void()>;

template <typename T, typename TIter>
void run_sequence(TIter items_begin, TIter items_end,
    std::function<VoidFunction(T item, int index, std::vector<T>& output_vector)> create_callback,
    size_t output_vector_size) {

  std::vector<T> output_vector(output_vector_size);

  int index = 0;
  for (auto iter = items_begin; iter != items_end; iter++, index++) {
    auto next = [](){};
    auto create_callback(*iter, index, output_vector);
  }
}

}

#endif
