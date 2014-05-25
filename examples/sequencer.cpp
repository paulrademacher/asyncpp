#include <iostream>
#include <vector>

#include "../async/async.hpp"

using namespace std;

int main(int argc, char *argv[]) {

  std::vector<int> ints { 0, 11, 22, 33, 44, 55, 66, 77, 88, 99 };

  auto callback = [](int item, int index, bool is_last_time,
      std::function<void(bool, async::ErrorCode)> callback_done) {

    printf("%d %d %d %d\n", item, index, is_last_time);

    if (item > 40)
      callback_done(true, async::OK);
  };

  auto final_callback = [](async::ErrorCode error) {
    printf("DONE\n");
  };

  async::sequencer<int, std::vector<int>::iterator>
      (ints.begin(), ints.end(), 5, callback,
          final_callback);
  return 0;
}

/*** UNIT TEST:

* No elements
* limit < # outstanding
* ==
* limit == #out + 1


****/
