#include <string>
#include "type_erasure/type_erasure.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>

template <typename C>
struct to_stringable_concept : C {
  virtual std::string to_string() const = 0;
};

template <typename M>
struct to_stringable_model : M {
  std::string to_string() const {
    return M::self().value().to_string();
  }
};

template <typename I>
struct to_stringable_interface : I {
  std::string to_string() const {
    namespace f = type_erasure::feature_support;
    return f::ifc_concept_ptr(*this)->to_string();
  }
};

struct to_stringable {
  template <typename C>
  using concept = to_stringable_concept<C>;

  template <typename M>
  using model = to_stringable_model<M>;

  template <typename I>
  using interface = to_stringable_interface<I>;
};

struct my_class {
  std::string to_string() const {
    return "I'm a my_class.";
  }
};

using to_stringable_t =
    type_erasure::any<to_stringable,
                      type_erasure::features::copy_constructible,
                      type_erasure::features::copy_assignable>;
std::ostream& operator<<(std::ostream& o, to_stringable_t const& x) {
  return o << x.to_string();
}

template <typename T>
std::vector<T> sorted(std::vector<T> x) {
  std::sort(x.begin(), x.end());
  return x;
}
int main() {
  std::vector<int> y{1,2,4,5,7,6,9};
  auto z = sorted(std::move(y));

}






