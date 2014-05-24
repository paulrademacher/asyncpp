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

/**
 * `limit` - the max number of items to process asynchronously.  If all items
 *      invoke their completion callbacks synchronously, then `limit` has no
 *      effect and the items proceed serially.  Set `limit` to 0 to indicate
 *      no limit on max number of asynchronous outstanding items.
 */
template <typename T, typename TIter, typename CustomData>
void sequencer(TIter items_begin, TIter items_end,
    unsigned int limit,
    CustomData& data,
    std::function<void(T item, int index, bool is_last_item, CustomData& data,
        std::function<void(bool keep_going, ErrorCode error)> callback_done)> callback,
    std::function<void(ErrorCode error, CustomData& data)> final_callback) {

  size_t num_items = items_end - items_begin;

  // If no items, invoke the final callback immediately with a success code.
  // This is easier than ensuring the complex logic below does the right thing for
  // an empty iterator.
  if (num_items == 0) {
    final_callback(async::OK, data);
    return;
  }

  if (limit == 0 || limit > num_items) {
    limit = num_items;
  }

  struct State {
    std::shared_ptr<State> keep_alive;
    TIter item_iter;
    unsigned int limit;
    unsigned int item_index = 0;
    unsigned int callbacks_outstanding = 0;
    unsigned int num_completed = 0;
    bool stop = false;
    std::function<void()> spawn_one;

    State() {
      priv::sequencer_state_count++;
    }

    ~State() {
      priv::sequencer_state_count--;
    }
  };

  auto state = std::make_shared<State>();

  state->keep_alive = state;
  state->item_iter = items_begin;
  state->limit = limit;

  state->spawn_one = [callback, &data, final_callback,
      &items_end, num_items, state]() mutable {

    state->callbacks_outstanding++;

    //    printf("spawning: index: %d \n", state->item_index);

    auto item = *state->item_iter;
    state->item_iter++;
    state->item_index++;

    callback(item, state->item_index - 1, state->item_index == num_items, data,
        [&data, final_callback, num_items, state] (bool keep_going, ErrorCode error) {

          state->callbacks_outstanding--;

          //          printf("outstanding: %d  limit: %d\n", state->callbacks_outstanding, state->limit);

          assert(state->callbacks_outstanding < state->limit);

          if (state->stop) {
            // We've already been instructed to stop by some earlier callback.
            return;
          }

          state->num_completed++;

          if (!keep_going) {
            // If callback says to stop, then stop.  But don't ever set this variable back to
            // false.
            state->stop = true;
          }

          if (state->stop || state->num_completed == num_items) {
            // All done.

            final_callback(error, data);

            // Break circular dependency, so state can release.
            //
            // The circular dependency is this:
            //   The singular State object contains the spawn_one std::function /
            //   lambda.  But that lambda captures `state` by value.  Because
            //   `state` is a shared_ptr, the capture creates a new shared_ptr,
            //   thereby incrementing the refcount of `state` by one.
            //   So even when all variables naturally go out of scope, the `state`
            //   object will still hold spawn_one, which holds `state`.
            //
            // Here we set spawn_one to an empty lambda, which clears the held
            // shared_ptr to state.
            //
            // Note that it is not sufficient to reset the `state` shared_ptr here.
            // That wouldn't break the circular ref.
            state->spawn_one = [](){};

            state->keep_alive.reset();
          } else if (state->callbacks_outstanding == state->limit - 1) {
            // We'd spawned as many items as our limit allows.  Since this callback
            // completed, we can spawn one more.
            if (state->item_index < num_items) {
              //              printf("spawn from CB\n");
              state->spawn_one();
            }
          }
        });
  };

  while (state->callbacks_outstanding < state->limit && !state->stop &&
      state->item_index < num_items) {
    //    printf("in main loop\n");
    state->spawn_one();
  }
}

}

#endif
