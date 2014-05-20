#include <iostream>
#include <vector>

#include "../async/async.hpp"

using namespace std;

int main(int argc, char *argv[]) {

  std::vector<int> ints { 0, 11, 22, 33, 44, 55, 66, 77, 88, 99 };
  auto user_callback = [](std::function<void(async::ErrorCode, int)> next) {
    next(async::OK, 23);
  };

  auto create_callback = [user_callback](int item, int index, std::vector<int>& output_vector) {
    printf("%d %d\n", item, index);
    return []() {  };
  };

  async::run_sequence<int>(ints.begin(), ints.end(), create_callback, ints.size());
  return 0;
}
