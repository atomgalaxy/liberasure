#include <vector>
#include <iostream>



namespace dims {

template <typename ValueType, int meters, int seconds, int kilograms>
struct dimensioned {
  ValueType value;
  friend bool operator==(dimensioned const& x, dimensioned const& y) {
    return x.value == y.value;
  }
  friend bool operator!=(dimensioned const& x, dimensioned const& y) {
    return !(x == y);
  }
  friend auto operator+(dimensioned const& x, dimensioned const& y) {
    return dimensioned{x.value + y.value};
  }
  friend auto operator-(dimensioned const& x, dimensioned const& y) {
    return dimensioned{x.value - y.value};
  }
  explicit operator ValueType() const {
    return value;
  }
  explicit operator bool() const { return bool(value); }
};
template <typename V1,
          int m1,
          int s1,
          int kg1,
          typename V2,
          int m2,
          int s2,
          int kg2>
auto operator*(dimensioned<V1, m1, s1, kg1> const& x,
               dimensioned<V2, m2, s2, kg2> const& y) {
  auto const result = x.value * y.value;
  return dimensioned<std::decay_t<decltype(result)>,
                     m1 + m2,
                     s1 + s2,
                     kg1 + kg2>{result};
}

template <typename ValueType>
using distance = dimensioned<ValueType, 1, 0, 0>;

template <typename ValueType>
using time = dimensioned<ValueType, 0, 1, 0>;

template <typename ValueType>
using mass = dimensioned<ValueType, 0, 0, 1>;

template <typename ValueType>
using velocity = dimensioned<ValueType, 1, -1, 0>;


};


int main() {

  {
    using namespace dims;
    distance<int> d{5};
    bool b = d;
    if (std::cin) {

    }
    std::cout << static_cast<decltype(d.value)>(d);

  }




  /// #####################################################
  {
  int const i = 0;
  auto j = i;
  int& k = j;
  auto m = k; // m is not a ref
  auto& n = m;

  decltype(auto) x = n;

  std::vector<int> v;

  auto const& u = v;
  for (auto item : v) {
    // BODY
  }
  }
}
