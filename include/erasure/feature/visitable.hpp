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

#pragma warning "Experimental!"

#include "../meta.hpp"

namespace erasure {
namespace features {
inline namespace {
namespace visitable_detail {

/* Visitable consists of two parts:
 * - the visitor interface
 *   - method of reach type, same return everywhere
 * So... visitable just has the ability to call .visit(correct_type) on our
 * visitor. The visitor only supplies methods
 */

template <typename Signature, typename Base>
struct add_signature;

template <typename ReturnType, typename... Params, typename Base>
struct add_signature<ReturnType(Params...), Base> : Base {
  virtual ReturnType _visit(Params...) = 0;
};

template <typename Signature, typename Base, typename Functor>
struct add_call;
template <typename ReturnType,
          typename... Params,
          typename Base,
          typename Functor>
struct add_call<ReturnType(Params...), Base, Functor> : Base {
  ReturnType _visit(Functor f, Params... xs) override final {
    return f(xs...);
  };
};

// template <typename Signature, typename Base, typename Functor>
// struct add_dispatch<


} // visitable_detail


/* VISITABLE */
template <typename FunctionSignature>
struct visitable;
template <typename ReturnType,
          typename... ParamTypeTypelists>
struct visitable<ReturnType(ParamTypeTypelists...)> {
  static_assert(
      meta::all_t<meta::is_typelist,
                  meta::typelist<std::decay_t<ParamTypeTypelists>...>>{},
      "All parameters to the function must be meta::typelist");

  // ParamTypeTypelist's cv-ref-ptr qualifications are preserved in the call.
  using visitor_type = visitor<ReturnType, ParamTypeTypelists...>;

  template <typename I>
  struct interface_ : I {};
  template <typename M>
  struct model_ : M {};
  template <typename C>
  struct concept_ : C {};

  template <typename Base>
  using interface = interface_<Base>;
  template <typename Base>
  using model = model_<Base>;
  template <typename Base>
  using concept = concept_<Base>;
};

} // visitable_detail
} // hide visitable_detail

using visitable_detail::visitable;

} // features
} // erasure
