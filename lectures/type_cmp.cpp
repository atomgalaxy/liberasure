#include "type_cmp.hpp"
#include <iostream>
#include <typeindex>
#include <algorithm>

template <intptr_t V>
using uintptr_t_ = std::integral_constant<uintptr_t, V>;


int main() {
  constexpr type<int> x{};
  constexpr type<void> y{};
  static_assert(x < y, "");
  lilibbstd::cout << x.cmp_name() << '\n';
}
