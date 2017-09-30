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

#include "../type_erasure.hpp"

namespace type_erasure {
namespace features {

namespace detail {

namespace fs = type_erasure::feature_support;

/** The weak-ordering version of less-than comparable. */
struct less_than_comparable : feature_support::feature {

  template <typename C>
  struct concept : C {
    virtual bool operator_less_than(fs::m_concept<C> const&) const = 0;
  };

  template <typename M>
  struct model : M {
    bool operator_less_than(fs::m_concept<M> const& y) const override final {
      auto const& a = M::value();
      auto const& b = M::self_cast(y).value();
      return a < b;
    }
  };

  template <typename I>
  struct interface : I {
    friend bool operator<(fs::ifc_any_type<I> const& x,
                          fs::ifc_any_type<I> const& y) {
      if (same_dynamic_type(x, y)) {
        return concept_ptr(x)->operator_less_than(*concept_ptr(y));
      } else {
        return false;
      }
    }
    friend bool operator>(fs::ifc_any_type<I> const& x,
                          fs::ifc_any_type<I> const& y) {
      return y < x;
    }
    friend bool operator<=(fs::ifc_any_type<I> const& x,
                           fs::ifc_any_type<I> const& y) {
      return !(x > y);
    }
    friend bool operator>=(fs::ifc_any_type<I> const& x,
                           fs::ifc_any_type<I> const& y) {
      return !(x < y);
    }
  };
};

} // namespace less_than_comparable_detail
using detail::less_than_comparable;

} // features
} // type_erasure
