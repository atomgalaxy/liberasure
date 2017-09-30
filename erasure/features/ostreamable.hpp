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
