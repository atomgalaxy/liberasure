#pragma once
#include "../type_erasure.hpp"

namespace type_erasure {
namespace features {
namespace value_equality_comparable_detail {

namespace f = type_erasure::feature_support;

template <typename T>
struct value_equality_comparable : feature_support::feature {
  template <typename C> struct concept : C { };
  template <typename M> struct model : M { };

  template <typename I>
  struct interface : I {
    friend bool operator==(f::ifc_any_type<I> const& x, T const& y) {
      if (auto ptr = target<T>(x)) {
        return *ptr == y;
      } else {
        return false;
      }
    }
    friend bool operator==(T const& x, f::ifc_any_type<I> const& y) {
      return y == x;
    }
    friend bool operator!=(f::ifc_any_type<I> const& x, T const& y) {
      return !(x == y);
    }
    friend bool operator!=(T const& x, f::ifc_any_type<I> const& y) {
      return !(x == y);
    }
  };
};
template <typename... Ts>
using equality_comparable_with = f::typelist<value_equality_comparable<Ts>...>;
}
using value_equality_comparable_detail::value_equality_comparable;
using value_equality_comparable_detail::equality_comparable_with;
}
}
