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

#include <iostream>

namespace erasure {
namespace features {

struct ostreamable : feature_support::feature {
  template <typename C>
  struct vtbl : C {
    using C::erase;
    virtual auto erase(tag_t<ostreamable>, std::ostream &) const
        -> std::ostream & = 0;
  };
  template <typename M>
  struct model : M {
    using M::erase;
    virtual auto erase(tag_t<ostreamable>, std::ostream &o) const
        -> std::ostream & final {
      return o << erasure::value(*this);
    }
  };
  template <typename I>
  struct interface : I {
    friend auto operator<<(std::ostream &o,
                           feature_support::ifc_any_type<I> const &x)
        -> std::ostream & {
      return erasure::call<ostreamable>(x, o);
    }
  };
};

} // namespace features
} // namespace erasure
