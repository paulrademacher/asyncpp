#include <iostream>

#include "../async/async.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  int count;

  auto whilst_test = [&count]() mutable {
    printf("    (%d < 3) == %s\n", count, count < 3 ? "true" : "false");
    return count < 3;
  };

  auto until_test = [&count]() mutable {
    printf("    (%d == 3) == %s\n", count, count == 3 ? "true" : "false");
    return count == 3;
  };

  auto func = [&count](std::function<void(async::ErrorCode error)> callback_done) {
    printf("        calling func: %d\n", count);
    count++;
    callback_done(async::OK);
  };

  auto final_callback = [](async::ErrorCode error) {
    printf("Done. In final_callback()\n");
  };

  printf("Using `whilst`:\n");
  count = 0;
  async::whilst(whilst_test, func, final_callback);
  printf("\n");

  printf("Using `doWhilst`:\n");
  count = 0;
  async::doWhilst(func, whilst_test, final_callback);
  printf("\n");

  printf("Using `until`:\n");
  count = 0;
  async::until(until_test, func, final_callback);
  printf("\n");

  printf("Using `doUntil`:\n");
  count = 0;
  async::doUntil(func, until_test, final_callback);
  printf("\n");

  return 0;
}
