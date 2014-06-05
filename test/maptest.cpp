#include "../async/async.hpp"

#define BOOST_TEST_MODULE MapTest
#include <boost/test/included/unit_test.hpp>
#include "test.hpp"

BEGIN_SEQUENCER_TEST(test1) {
  bool callback_called = false;
  std::vector<int> data { 1, 2, 3, 4 };
  async::MapCallback<int> callback = [](int value, async::TaskCallback<int> callback) {
    callback(async::OK, value * value);
  };
  auto final_callback = [&callback_called](async::ErrorCode error, std::vector<int> results) {
    callback_called = true;

    std::vector<int> expected { 1, 4, 9, 16 };
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results),
        begin(expected), end(expected));
    BOOST_CHECK_EQUAL(error, async::OK);
    BOOST_CHECK(async::get_sequencer_state_count() > 0);
  };
  async::map<int>(data, callback, final_callback);
  BOOST_CHECK(callback_called);

  END_SEQUENCER_TEST();
}

BEGIN_SEQUENCER_TEST(test_map_with_limits) {
  bool callback_called = false;
  std::vector<int> data { 1, 2, 3, 4 };
  int limit = 2;  // Process at most 2 at once.
  async::map<int>(data, [](int value, async::TaskCallback<int> callback) {
        callback(async::OK, value * value);
      },
      [&callback_called](async::ErrorCode error, std::vector<int> results) {
        callback_called = true;

        std::vector<int> expected { 1, 4, 9, 16 };
        BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results),
            begin(expected), end(expected));
        BOOST_CHECK_EQUAL(error, async::OK);
        BOOST_CHECK(async::get_sequencer_state_count() > 0);
      },
      limit);
  BOOST_CHECK(callback_called);

  END_SEQUENCER_TEST();
}

BEGIN_SEQUENCER_TEST(test_map_with_error) {
  bool callback_called = false;
  std::vector<int> data { 1, 2, 3, 4 };
  int limit = 2;  // Process at most 2 at once.
  async::map<int>(data, [](int value, async::TaskCallback<int> callback) {
        if (value == 3) {
          callback(async::FAIL, -1);
        } else {
          callback(async::OK, value * value);
        }
      },
      [&callback_called](async::ErrorCode error, std::vector<int> results) {
        callback_called = true;

        std::vector<int> expected { 1, 4, -1, 0 };
        BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results),
            begin(expected), end(expected));
        BOOST_CHECK_EQUAL(error, async::FAIL);
        BOOST_CHECK(async::get_sequencer_state_count() > 0);
      },
      limit);
  BOOST_CHECK(callback_called);

  END_SEQUENCER_TEST();
}

BEGIN_SEQUENCER_ASIO_TEST(test_map_async) {
  auto data = new std::vector<int> {0, 1, 2, 3, 4, 5};

  async::MapCallback<int> func = make_task_callback_square(io_service, timers, 1);

  async::map<int>(*data, func,
      [](async::ErrorCode error, std::vector<int> results) {
        std::vector<int> expected {0, 1, 4, 9, 16, 25};
        BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results),
            begin(expected), end(expected));
        BOOST_CHECK_EQUAL(error, async::OK);
      }, 1);

  END_SEQUENCER_ASIO_TEST(data);
}

BEGIN_SEQUENCER_ASIO_TEST(test_map_async2) {
  auto data = new std::vector<int> {0, 1, 2, 3, 4, 5};

  async::MapCallback<int> func = make_task_callback_square(io_service, timers, 1);

  async::map<int>(*data, func,
      [](async::ErrorCode error, std::vector<int> results) {
        std::vector<int> expected {0, 1, 4, 9, 16, 25};
        BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results),
            begin(expected), end(expected));
        BOOST_CHECK_EQUAL(error, async::OK);
      }, 3);

  END_SEQUENCER_ASIO_TEST(data);
}

BOOST_AUTO_TEST_CASE(map_test) {
}
