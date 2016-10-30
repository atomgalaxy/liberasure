#include <random>
#include <cstdlib>
#include <iostream>

int main() {
  srand(std::random_device{}());

  for (auto i = 0; i < 10; ++i) {
    std::cout << rand() << '\n';
  }

}
