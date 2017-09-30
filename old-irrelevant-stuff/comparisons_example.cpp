/*
 * Copyright 2015, 2016 Gašper Ažman
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
