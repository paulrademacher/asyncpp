// Common test utils and macros.

#define BEGIN_SEQUENCER_TEST(name)                                    \
  BOOST_AUTO_TEST_CASE(name) {                                        \
    BOOST_REQUIRE_EQUAL(async::get_sequencer_state_count(), 0);       \
    auto time_start = boost::posix_time::second_clock::local_time();  \
    // Run the sequencer code in its own scope, before then checking  \
    // get_sequencer_state_count()==0.

#define END_SEQUENCER_TEST()                                \
  }                                                         \
  BOOST_CHECK_EQUAL(async::get_sequencer_state_count(), 0);


// ASIO helpers --------------------------------------------------


#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

using boost::posix_time::seconds;

#define BEGIN_SEQUENCER_ASIO_TEST(name)                                 \
  BOOST_AUTO_TEST_CASE(name) {                                          \
    BOOST_REQUIRE_EQUAL(async::get_sequencer_state_count(), 0);         \
    boost::asio::io_service io_service;                                 \
    auto time_start = boost::posix_time::second_clock::local_time();    \
    std::vector<std::unique_ptr<boost::asio::deadline_timer>> timers;   \
    // Run the sequencer code in its own scope, before then checking    \
    // get_sequencer_state_count()==0.

#define END_SEQUENCER_ASIO_TEST(TO_DELETE)                        \
    io_service.run();                                             \
    delete TO_DELETE;                                             \
  }                                                               \
  BOOST_CHECK_EQUAL(async::get_sequencer_state_count(), 0);

#define CHECK_TIME_LAPSE(expected)                                      \
  {                                                                     \
  auto time_diff = boost::posix_time::second_clock::local_time() - time_start; \
  /*printf("time_diff: %d\n", time_diff.total_milliseconds());   */     \
  BOOST_CHECK(time_diff.total_milliseconds() > expected - 100 && \
      time_diff.total_milliseconds() < expected + 100); \
  }

void invoke_or_defer(boost::asio::io_service& io_service,
    std::vector<std::unique_ptr<boost::asio::deadline_timer>>& timers,
    int timer_seconds, std::function<void()> func) {
  if (timer_seconds >= 0) {
    // Use boost::asio.
    printf("task w/delayed callback: %d seconds\n", timer_seconds);
    timers.emplace_back(new boost::asio::deadline_timer(io_service));
    timers.back()->expires_from_now(seconds(timer_seconds));
    timers.back()->async_wait([=] (const boost::system::error_code& error) {
          func();
        });
  } else {
    // Negative seconds means return a result immediately and do not
    // defer this result.
    func();
  }
}

async::Task<int> make_task_callback_no_input(boost::asio::io_service& io_service,
    std::vector<std::unique_ptr<boost::asio::deadline_timer>>& timers,
    int timer_seconds, int output) {
  return [=, &io_service, &timers](async::TaskCallback<int> callback) {
    invoke_or_defer(io_service, timers, timer_seconds,
        [=]() { callback(async::OK, output); });
  };
}

async::MapCallback<int> make_task_callback_square(boost::asio::io_service& io_service,
    std::vector<std::unique_ptr<boost::asio::deadline_timer>>& timers,
    int timer_seconds) {
  return [=, &io_service, &timers](int input, async::TaskCallback<int> callback) {
    invoke_or_defer(io_service, timers, timer_seconds,
        [=]() { callback(async::OK, input * input); });
  };
}
