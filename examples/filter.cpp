#include <iostream>
#include <vector>

#include "../async/async.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  std::vector<int> data { 1, 2, 3, 4, 5, 6, 7 };

  auto filter_test = [](int item, async::BoolCallback callback) {
    // Filter out odd items.
    callback((item % 2) == 0);
  };

  auto final_callback = [](vector<int> results) {
    for (auto result : results) {
      cout << result << " ";
    }
    cout << endl;
  };

  cout << "Filter:" << endl;
  async::filter<int>(data, filter_test, final_callback);

  cout << endl;
  cout << "Reject:" << endl;
  async::reject<int>(data, filter_test, final_callback);

  return 0;
}
