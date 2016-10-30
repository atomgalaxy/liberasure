#include "comparisons.hpp"
#include <cassert>

int main() {
  using comparison_chaining::cmp;
  auto x = 10;
  assert(cmp % -5 < x);
  assert(cmp % x > -5);
  assert(!(cmp % x < -5));
  assert(cmp % 5 < x <= (cmp(10) <= x < 15));
  assert(cmp % 5 < x <= 10 <= x < 15);
  assert(cmp % 5 < x < 11);
  auto y = -10;
  assert(cmp % -15 < y < -3);
}
