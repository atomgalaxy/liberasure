#include "erasure/meta.hpp"

namespace erm = erasure::meta;
void test_compose() {
  static_assert(!erm::compose<erm::not_, erm::is_same>::type<int, int>{}, "");
  static_assert(
      erm::compose<erm::not_, erm::compose<erm::not_, erm::is_same>::type>::
          type<int, int>{},
      "");
  static_assert(erm::compose<erm::not_, erm::not_>::type<std::true_type>{}, "");
}

void test_cons() {
  static_assert(erm::typelist<int>{} == erm::cons_t<int>{},
                "A cons with a single parameter should cons to an empty list.");
  static_assert(erm::typelist<int, long, char>{} ==
                    erm::cons_t<int, erm::cons_t<long, erm::cons_t<char>>>{},
                "Cons builds erm::typelists correctly.");
}

void test_take_1_t() {
  static_assert(erm::typelist<int>{} == erm::take_1_t<erm::typelist<int>>{},
                "");
  static_assert(erm::typelist<int>{} ==
                    erm::take_1_t<erm::typelist<int, long, char>>{},
                "");
  static_assert(erm::typelist<>{} == erm::take_1_t<erm::typelist<>>{}, "");
}

void test_concatenate() {
  static_assert(
      erm::typelist<>{} == erm::concatenate_t<>{},
      "Concatenation of no erm::typelists is the unit for concatenation.");
  static_assert(erm::typelist<>{} ==
                    erm::concatenate_t<erm::typelist<>, erm::typelist<>>{},
                "erm::typelist<> is the unit for concatenation.");
  static_assert(
      erm::typelist<int, long, char>{} ==
          erm::concatenate_t<erm::typelist<int>, erm::typelist<long, char>>{},
      "Concatenate concatenates.");
  static_assert(erm::typelist<int, long, char>{} ==
                    erm::concatenate_t<erm::typelist<int>, erm::typelist<long>,
                                       erm::typelist<char>>{},
                "");
}

namespace test_forward_cast {
struct sx {};
struct sy : sx {};

template <typename T>
struct undef;
template <typename T>
using result = decltype(erm::forward_cast<sx>(std::declval<T>()));

void test() {
  static_assert(std::is_same_v<sx &, result<sy &>>,
                "mutable refs count as mutable");
  static_assert(std::is_same_v<sx &&, result<sy &&>>,
                "mutable rvalue-refs count as mutable rvalue-refs");
  static_assert(std::is_same_v<sx const &, result<sy const &>>,
                "const lvalue-refs count as const lvalue refs");
  static_assert(std::is_same_v<sx const &&, result<sy const &&>>,
                "const rvalue-refs count as const rvalue refs");
}
} // namespace test_forward_cast

// JUST SO IT COMPILES
int main() {
  test_compose();
  test_cons();
  test_take_1_t();
  test_concatenate();
}
