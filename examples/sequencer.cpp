#include <iostream>
#include <vector>

#include "../async/async.hpp"

using namespace std;

int main(int argc, char *argv[]) {

  std::vector<int> ints { 0, 11, 22, 33, 44, 55, 66, 77, 88, 99 };

  int custom_data = 999;

  auto callback = [](int item, int index, bool is_last_time, int& data,
      std::function<void(bool, async::ErrorCode)> callback_done) {

    printf("%d %d %d %d\n", item, index, is_last_time, data);

    if (item > 40)
      callback_done(true, async::OK);
  };

  auto final_callback = [](async::ErrorCode error, int& data) {
    printf("DONE\n");
  };

  async::run_sequence<int, std::vector<int>::iterator, int>
      (ints.begin(), ints.end(), 5, custom_data, callback,
          final_callback);
  return 0;
}

/*** UNIT TEST:

* No elements
* limit < # outstanding
* ==
* limit == #out + 1


****/
