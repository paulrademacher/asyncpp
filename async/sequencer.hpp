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
template <typename T, typename TIter>
void sequencer(TIter items_begin, TIter items_end,
    unsigned int limit,
    std::function<void(T item, int index, bool is_last_item,
        std::function<void(bool keep_going, ErrorCode error)> callback_done)> callback,
    std::function<void(ErrorCode error)> final_callback) {

  // If no items, invoke the final callback immediately with a success code.
  // This is easier than ensuring the complex logic below does the right thing for
  // an empty iterator.
  if (items_begin == items_end) {
    final_callback(async::OK);
    return;
  }

  struct State {
    std::shared_ptr<State> keep_alive;
    TIter item_iter;
    TIter items_end;
    unsigned int limit;
    unsigned int item_index = 0;
    unsigned int callbacks_outstanding = 0;
    bool stop = false;
    bool in_main_loop;
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
  state->items_end = items_end;
  state->limit = limit;

  state->spawn_one = [callback, final_callback,
      &items_end, state]() mutable {

    state->callbacks_outstanding++;

    auto item = *state->item_iter;
    bool is_last_item = state->item_iter == state->items_end;
    state->item_iter++;
    state->item_index++;

    callback(item, state->item_index - 1, is_last_item,
        [final_callback, state] (bool keep_going, ErrorCode error) {

          state->callbacks_outstanding--;

          assert(state->limit == 0 || state->callbacks_outstanding < state->limit);

          if (state->stop) {
            // We've already been instructed to stop by some earlier callback.
            return;
          }

          if (!keep_going) {
            // If callback says to stop, then stop.  But don't ever set this variable back to
            // false.
            state->stop = true;
          }

          if (state->stop ||
              (state->callbacks_outstanding == 0 && state->item_iter == state->items_end)) {
            // All done.

            final_callback(error);

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
          } else if (state->limit != 0 && state->callbacks_outstanding == state->limit - 1) {
            // We'd spawned as many items as our limit allows.  Since this callback
            // completed, we can spawn one more.
            if (state->item_iter != state->items_end) {
              if (!state->in_main_loop) {
                // We're not inside the main loop, which means we are currently in an
                // asynchronous callback.  Spawn the next item in the sequence directly
                // from here.  Otherwise, if we're inside the main loop, the next item
                // will get spawned once this function returns.
                state->spawn_one();

                // TODO: For better stack management, this should be a loop and not just a
                // single spawn, applying the same logic to not spawn internally but from
                // the parent loop.  This way, if an async callback contains a lot of sync tasks,
                // we won't blow the stack.
              }
            }
          }
        });
  };

  state->in_main_loop = true;
  while ((state->limit == 0 || state->callbacks_outstanding < state->limit) &&
      !state->stop &&
      state->item_iter != state->items_end) {
    state->spawn_one();
  }
  state->in_main_loop = false;
}

}

#endif
