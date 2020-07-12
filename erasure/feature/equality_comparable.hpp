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
struct equality_comparable : feature_support::feature {
  // vtbl
  template <typename C>
  struct vtbl : C {
    using C::erase;
    virtual auto erase(erasure::tag_t<equality_comparable>,
                       feature_support::m_vtbl<C> const &y) const -> bool = 0;
  };

  template <typename M>
  struct model : M {
      using M::erase;
    // precondition: same type, ensured by interface
    auto erase(erasure::tag_t<equality_comparable>, feature_support::m_vtbl<M> const& y) const -> bool final {
      auto const& a = M::value();
      auto const& b = M::self_cast(y).value();
      return a == b;
    }
  };

  template <typename I>
  struct interface : I {
    friend auto operator==(feature_support::ifc_any_type<I> const& x,
                           feature_support::ifc_any_type<I> const& y) -> bool {
      if (same_dynamic_type(x, y)) {
        return concept_ptr(x)->erase(tag<equality_comparable>, *concept_ptr(y));
      } else {
        return false;
      }
    }
    friend auto operator!=(feature_support::ifc_any_type<I> const& x,
                           feature_support::ifc_any_type<I> const& y) -> bool {
      return !(x == y);
    }
  };
};

}
}
