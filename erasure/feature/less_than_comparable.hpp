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

namespace detail {

namespace fs = erasure::feature_support;

/** The weak-ordering version of less-than comparable. */
struct less_than_comparable : feature_support::feature {

  template <typename C>
  struct vtbl : C {
    using C::erase;
    virtual auto erase(tag_t<less_than_comparable>, fs::m_vtbl<C> const&) const -> bool = 0;
  };

  template <typename M>
  struct model : M {
    using M::erase;
    auto erase(tag_t<less_than_comparable>, fs::m_vtbl<M> const &y) const -> bool final {
      auto const& a = M::value();
      auto const& b = M::self_cast(y).value();
      return a < b;
    }
  };

  template <typename I>
  struct interface : I {
    friend auto operator<(fs::ifc_any_type<I> const& x,
                          fs::ifc_any_type<I> const& y) -> bool {
      if (same_dynamic_type(x, y)) {
        return concept_ptr(x)->erase(tag<less_than_comparable>, *concept_ptr(y));
      } else {
        return false;
      }
    }
    friend auto operator>(fs::ifc_any_type<I> const& x,
                          fs::ifc_any_type<I> const& y) -> bool {
      return y < x;
    }
    friend auto operator<=(fs::ifc_any_type<I> const& x,
                           fs::ifc_any_type<I> const& y) -> bool {
      return !(x > y);
    }
    friend auto operator>=(fs::ifc_any_type<I> const& x,
                           fs::ifc_any_type<I> const& y) -> bool {
      return !(x < y);
    }
  };
};

} // namespace less_than_comparable_detail
using detail::less_than_comparable;

} // features
} // erasure
