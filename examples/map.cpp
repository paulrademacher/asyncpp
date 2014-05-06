#include <iostream>
#include <vector>

#include "../async.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  std::vector<int> objects { 1, 2, 3, 4 };

  async::map<int>(objects,
      [](int object, async::TaskCallback<int> callback) {
        callback(async::OK, object * object);
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
