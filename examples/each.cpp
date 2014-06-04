#include <iostream>
#include <vector>

#include "../async/async.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  std::vector<int> data { 1, 2, 3, 4, 5, 6, 7 };

  async::each<int>(data,
      [](int value, async::ErrorCodeCallback callback) {
        cout << value << endl;
        callback(value < 5 ? async::OK : async::FAIL);
      },
      [](async::ErrorCode error) {
        cout << "Done.  error code=" << error << endl;
      });

  return 0;
}
