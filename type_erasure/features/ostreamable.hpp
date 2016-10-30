#pragma once

#include "../type_erasure.hpp"

#include <iostream>

namespace type_erasure {
namespace features {

struct ostreamable : feature_support::feature {
  template <typename C>
  struct concept : C {
    virtual std::ostream& operator_left_shift(std::ostream&) const = 0;
  };
  template <typename M>
  struct model : M {
    virtual std::ostream& operator_left_shift(
        std::ostream& o) const override final {
      return o << M::self().value();
    }
  };
  template <typename I>
  struct interface : I {
    friend std::ostream& operator<<(
        std::ostream& o, feature_support::ifc_any_type<I> const& x) {
      return concept_ptr(x)->operator_left_shift(o);
    }
  };
};

}  // features
}  // type_erasure
