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

#pragma once
#include "erasure/erasure.hpp"

namespace erasure {
namespace features {
namespace value_equality_comparable_detail {

namespace f = erasure::feature_support;

template <typename T>
struct value_equality_comparable : feature_support::feature {
  template <typename C>
  using vtbl = C;
  template <typename M>
  using model = M;

  template <typename I>
  struct interface : I {
    friend auto operator==(f::ifc_any_type<I> const& x, T const& y) -> bool {
      if (auto ptr = target<T>(x)) {
        return *ptr == y;
      } else {
        return false;
      }
    }
    friend auto operator==(T const& x, f::ifc_any_type<I> const& y) -> bool {
      return y == x;
    }
    friend auto operator!=(f::ifc_any_type<I> const& x, T const& y) -> bool {
      return !(x == y);
    }
    friend auto operator!=(T const& x, f::ifc_any_type<I> const& y) -> bool {
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
