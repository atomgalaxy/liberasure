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

template <typename ReturnType>
struct const_dereferenceable : feature_support::feature {
  template <typename C>
  struct vtbl : C {
    virtual auto operator_const_dereference() const -> ReturnType = 0;
  };
  template <typename M>
  struct model : M {
    virtual auto operator_const_dereference() const -> ReturnType override final {
      return *M::value();
    }
  };
  template <typename I>
  struct interface : I {
    auto operator*() const -> ReturnType {
      namespace f = erasure::feature_support;
      return f::ifc_concept_ptr(*this)->operator_const_dereference();
    }
  };
};
template <typename ReturnType>
using dereferenceable = const_dereferenceable<ReturnType>;

template <typename ReturnType>
struct mutably_dereferenceable : feature_support::feature {
  template <typename C>
  struct vtbl : C {
    virtual auto operator_mutable_dereference() -> ReturnType = 0;
  };
  template <typename M>
  struct model : M {
    virtual auto operator_mutable_dereference() -> ReturnType override final {
      return *M::value();
    }
  };
  template <typename I>
  struct interface : I {
    auto operator*() -> ReturnType {
      namespace f = erasure::feature_support;
      return f::ifc_concept_ptr(*this)->operator_mutable_dereference();
    }
  };
};

}  // features
}  // erasure
