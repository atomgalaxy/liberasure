#pragma once

#include "../type_erasure.hpp"

namespace type_erasure {
namespace features {
namespace equality_comparable_detail {

namespace f = type_erasure::feature_support;
struct equality_comparable : feature_support::feature {
  // concept
  template <typename C>
  struct concept : C {
    virtual bool operator_equals(f::m_concept<C> const& y) const = 0;
  };

  template <typename M>
  struct model : M {
    // precondition: same type, ensured by interface
    bool operator_equals(f::m_concept<M> const& y) const override final {
      auto const& a = M::value();
      auto const& b = M::self_cast(y).value();
      return a == b;
    }
  };

  template <typename I>
  struct interface : I {
    friend bool operator==(f::ifc_any_type<I> const& x,
                           f::ifc_any_type<I> const& y) {
      if (same_dynamic_type(x, y)) {
        return concept_ptr(x)->operator_equals(*concept_ptr(y));
      } else {
        return false;
      }
    }
    friend bool operator!=(f::ifc_any_type<I> const& x,
                           f::ifc_any_type<I> const& y) {
      return !(x == y);
    }
  };
};

}
using equality_comparable_detail::equality_comparable;
}
}
