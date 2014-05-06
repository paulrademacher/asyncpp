#include <iostream>
#include <vector>

#include "../async.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  async::SeriesTaskVector<int> tasks {
    [](async::TaskCallback<int> callback) {
      callback(async::OK, 1);
    },
    [](async::TaskCallback<int> callback) {
      callback(async::OK, 2);
    },
    [](async::TaskCallback<int> callback) {
      callback(async::OK, 3);
    }
  };

  async::series<int>(tasks, [](async::ErrorCode error, vector<int> results) {
        cout << "Error: " << error << endl;
        for (auto result : results) {
          cout << result << " ";
        }
        cout << endl;
      });

  return 0;
}
