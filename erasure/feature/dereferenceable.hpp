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
struct dereference_tag {
  template <template <typename> class T, typename U>
  struct importer : T<U> {
    using T<U>::operator*;
    using U::operator*;
  };
};

#define ERASURE_DEREF_DEF(name, constness)                                     \
  template <typename ReturnType>                                               \
  struct name : feature_support::feature {                                     \
    using provides = dereference_tag;                                          \
    template <typename C>                                                      \
    struct vtbl : C {                                                          \
      using C::erase;                                                          \
      virtual auto erase(erasure::tag_t<name>) constness -> ReturnType = 0;    \
    };                                                                         \
    template <typename M>                                                      \
    struct model : M {                                                         \
      using M::erase;                                                          \
      virtual auto erase(erasure::tag_t<name>) constness -> ReturnType final { \
        return *erasure::value(std::forward<M constness>(*this));                                          \
      }                                                                        \
    };                                                                         \
    template <typename I>                                                      \
    struct interface : I {                                                     \
      auto operator*() constness -> ReturnType {                               \
        namespace f = erasure::feature_support;                                \
        return erasure::call<name>(*this);                                     \
      }                                                                        \
    };                                                                         \
  };                                                                           \
  static_assert(true, "require semicolon")

ERASURE_DEREF_DEF(const_dereferenceable, const);
ERASURE_DEREF_DEF(mutably_dereferenceable, );
#undef ERASURE_DEREF_DEF
template <typename ReturnType>
using dereferenceable = const_dereferenceable<ReturnType>;

} // namespace features
} // namespace erasure
