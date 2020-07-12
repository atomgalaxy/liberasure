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
                       erasure::vtbl<C> const &y) const -> bool = 0;
  };

  template <typename M>
  struct model : M {
    using M::erase;
    // precondition: same type, ensured by interface
    auto erase(erasure::tag_t<equality_comparable>,
               erasure::vtbl<M> const &y) const -> bool final {
      auto const &a = erasure::value(*this);
      auto const &b = erasure::value(erasure::self_cast(*this, y));
      return a == b;
    }
  };

  template <typename I>
  struct interface : I {
    friend auto operator==(erasure::ifc<I> const &x, erasure::ifc<I> const &y)
        -> bool {
      if (same_dynamic_type(x, y)) {
        return erasure::call<equality_comparable>(x, *erasure::concept_ptr(y));
      } else {
        return false;
      }
    }
    friend auto operator!=(erasure::ifc<I> const &x, erasure::ifc<I> const &y)
        -> bool {
      return !(x == y);
    }
  };
};

} // namespace features
} // namespace erasure
