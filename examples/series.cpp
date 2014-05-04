#include <iostream>
#include <vector>

#include "../async.hpp"

int main(int argc, char *argv[]) {
  async::SeriesTaskVector<int> tasks {
    [](async::SeriesCallback<int> callback) {
      callback(async::OK, 1);
    },
    [](async::SeriesCallback<int> callback) {
      callback(async::OK, 3);
    },
    [](async::SeriesCallback<int> callback) {
      callback(async::OK, 5);
    },
    [](async::SeriesCallback<int> callback) {
      callback(async::OK, 7);
    }
  };

  async::series<int>(tasks, [](async::ErrorCode error, std::vector<int> results) {
        std::cout << "Error: " << error << std::endl;
        for (auto result : results) {
          std::cout << result << " ";
        }
        std::cout << std::endl;
      });

  return 0;
}
