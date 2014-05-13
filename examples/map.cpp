#include <iostream>
#include <vector>

#include "../async.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  std::vector<int> objects { 1, 2, 3, 4, 5, 6, 7 };

  async::map<int>(objects,
      [](int value, async::TaskCallback<int> callback) {
        callback(async::OK, value * value);
      },
      [](async::ErrorCode error, vector<int> results) {
        cout << "Error: " << error << endl;
        for (auto result : results) {
          cout << result << " ";
        }
        cout << endl;
      });

  return 0;
}
