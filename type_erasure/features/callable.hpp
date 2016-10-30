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
namespace callable_detail {

struct provides_call_operator {
  template <template <typename> class T, typename U>
  struct model_name_unhider : T<U> {
    using T<U>::operator_call;
    using U::operator_call;
  };
  template <template <typename> class T, typename U>
  struct interface_name_unhider : T<U> {
    using T<U>::operator();
    using U::operator();
  };
};

}

// provide the correct form for pattern matching
template <typename Signature>
struct callable;

template <typename Return, typename... Args>
struct callable<Return(Args...)> : feature_support::feature {
  using provides = callable_detail::provides_call_operator;

  template <typename C>
  struct concept : C {
    virtual Return operator_call(Args... args) const = 0;
  };

  template <typename M>
  struct model : M {
    Return operator_call(Args... args) const override final {
      return M::value()(args...);
    }
  };

  template <typename I>
  struct interface : I {
    Return operator()(Args... args) const {
      namespace f = feature_support;
      return f::ifc_concept_ptr(*this)->operator_call(
          std::forward<Args>(args)...);
    }
  };
};

// provide the correct form for pattern matching
template <typename Signature>
struct mutably_callable;

template <typename Return, typename... Args>
struct mutably_callable<Return(Args...)> : feature_support::feature {
  using provides = callable_detail::provides_call_operator;

  template <typename C>
  struct concept : C {
    virtual Return operator_call(Args... args) = 0;
  };

  template <typename M>
  struct model : M {
    Return operator_call(Args... args) override final {
      return M::value()(args...);
    }
  };

  template <typename I>
  struct interface : I {
    Return operator()(Args... args) {
      namespace f = feature_support;
      return f::ifc_concept_ptr(*this)->operator_call(args...);
    }
  };
};

/**
 * The default size for the buffer for function is vtable_ptr + function
 * pointer + this state.
 */
template <typename Signature, size_t BufferSize = 3 * sizeof(void*)>
using function = feature_support::typelist<buffer_size<BufferSize>,
                                           callable<Signature>,
                                           move_constructible,
                                           copy_constructible>;

} // features
} // type_erasure
