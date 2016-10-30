#include "type_erasure/features/callable.hpp"
#include "type_erasure/features/regular.hpp"
#include "type_erasure/type_erasure.hpp"

#include <memory>
#include <tuple>
#include <cassert>
#include <functional>

struct can_const_call {
  int operator()() const { return 1; }
  int operator()(int) const { return 2; }

  friend bool operator==(can_const_call const&, can_const_call const&) {
    return true;
  }
};

struct can_mutably_call {
  int x = 0;
  int operator()() { return ++x; }
  int operator()(int) { return --x; }

  friend bool operator==(can_mutably_call const&, can_mutably_call const&) {
    return true;
  }
};

void test_const_callable() {
  using type_erasure::features::regular;
  using type_erasure::features::callable;
  using type_erasure::features::copyable;
  using type_erasure::features::buffer_size;
  using type_erasure::make_any;
  using type_erasure::make_any_like;

  auto x = make_any<regular,
                    callable<int()>,
                    callable<int(int)>,
                    buffer_size<8 + 8>>(can_const_call{});
  assert(x() == 1);
  assert(x(1) == 2);
  can_const_call cc{};
  auto y = make_any_like<decltype(x)>(std::ref(cc));
  assert(y(1) == 2);
}

void test_mutably_callable() {
  using type_erasure::features::regular;
  using type_erasure::features::mutably_callable;
  using type_erasure::make_any;

  auto x =
      make_any<regular,
               mutably_callable<int()>,
               mutably_callable<int(int)>>(
          can_mutably_call{});
  assert(x() == 1);
  assert(x() == 2);
  assert(x() == 3);
  assert(x(1) == 2);
  assert(x(1) == 1);
}

int main() {
  test_const_callable();
  test_mutably_callable();
}
