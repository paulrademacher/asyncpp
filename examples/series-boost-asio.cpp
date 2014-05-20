#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include "../async/async.hpp"

using boost::posix_time::seconds;
using namespace std;

int main(int argc, char *argv[]) {
  boost::asio::io_service io_service;
  boost::asio::deadline_timer timer(io_service);

  async::SeriesTaskVector<int> tasks {
    [&timer](async::TaskCallback<int> callback) {
      timer.expires_from_now(seconds(1));
      timer.async_wait([=] (const boost::system::error_code& error) {
            cout << "Task 1 completed" << endl;
            callback(async::OK, 1);
          });
    },

    [&timer](async::TaskCallback<int> callback) {
      timer.expires_from_now(seconds(1));
      timer.async_wait([=] (const boost::system::error_code& error) {
            cout << "Task 2 completed" << endl;
            callback(async::OK, 2);
          });
    },

    [&timer](async::TaskCallback<int> callback) {
      timer.expires_from_now(seconds(1));
      timer.async_wait([=] (const boost::system::error_code& error) {
            cout << "Task 3 completed" << endl;
            callback(async::OK, 3);
          });
    },
  };

  async::series<int>(tasks, [](async::ErrorCode error, vector<int> results) {
        cout << "----------\nAll done" << endl;
        cout << "Error: " << error << endl;
        for (auto result : results) {
          cout << result << " ";
        }
        cout << endl;
      });

  io_service.run();
  return 0;
}
