#ifndef TYPE_ERASURE__FEATURES__VISITABLE_HPP
#define TYPE_ERASURE__FEATURES__VISITABLE_HPP

#pragma warning "Experimental!"

#include "../meta.hpp"

namespace type_erasure {
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
} // type_erasure

#endif
