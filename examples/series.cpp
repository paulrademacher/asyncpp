#include <iostream>
#include <vector>

#include "../async/async.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  // Series with results.
  async::TaskVector<int> tasks {
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

  // Series without results.
  async::ResultlessTaskVector tasks_no_results {
    [](async::ErrorCodeCallback callback) {
      cout << 1 << endl;
      callback(async::OK);
    },
    [](async::ErrorCodeCallback callback) {
      cout << 2 << endl;
      callback(async::OK);
    },
    [](async::ErrorCodeCallback callback) {
      cout << 3 << endl;
      callback(async::OK);
    },
  };

  async::series_no_results(tasks_no_results, [](async::ErrorCode error) {
        cout << "Error: " << error << endl;
        cout << endl;
      });

  return 0;
}
