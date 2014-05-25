#include <iostream>

#include "../async/async.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  int count = 0;

  auto test = [&count]() mutable -> bool {
    return count++ < 100;
  };

  auto func = [&count](std::function<void(async::ErrorCode error)> callback_done) {
    printf("%d ", count);
    callback_done(async::OK);
  };

  auto final_callback = [](async::ErrorCode error) {
    printf("\nDONE\n");
  };

  async::whilst(test, func, final_callback);

  return 0;
}
