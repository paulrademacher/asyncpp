#include <iostream>

#include "../async/async.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  int count = 0;

  auto func = [&count](std::function<void(async::ErrorCode error)> callback) {
    printf("%d\n", count++);
    callback(async::OK);
  };

  async::ntimes(10, func);

  return 0;
}
