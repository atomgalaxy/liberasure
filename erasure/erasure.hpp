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
/**
 * @file erasure.hpp
 * The general library for constructing type-erased types.
 *
 * INTRODUCTION
 * ============
 *
 * This library provides the type constructor `any<>` and a number of features
 * and feature-sets for it. It also provides means of writing new features to
 * make quick and painless.
 *
 * The term _type-erased_ refers to the fact that the interface of the type is
 * retained, but the name and the implementation of the type is not. If it
 * sounds similar to the 'abstract interface' and 'implementation' in designs
 * using inheritance, it is. The difference is that inheritance-based designs
 * require an explicit derivation from the interface, and pessimize all access
 * to any object having a vtable even when not required (modulo compiler
 * optimizations such as devirtualization). In addition, they expect an
 * explicit .clone() or .copy() method to make a copy of the object, and that
 * copy will then usually presume to be stored on the heap. In type-erasure
 * based designs, the normal copy construction, copy assignment, move
 * construction and move assignment are given their usual meanings, so copies
 * can be enacted in the same way as for other types, with the same syntax. It
 * is thus the solution to truly polymorphic, regular types with no abstraction
 * penalty when you do not need the polymorphism, since no inheritance (on the
 * polymorphic types) is necessary.
 *
 * To use the library, you only need know how to use the any constructor and
 * which features are available. All following information is for people who
 * wish to extend the library and understand its design.
 *
 * If you just want to write a feature, the easiest way is to jump to the
 * 'feature class interface' part, then see an example (studying callable,
 * equality_comparable and value_equality_comparable should suffice).
 *
 *
 * THE CONSTRUCTION MECHANISM
 * --------------------------
 *
 * An any_t comprises three parts: the physical interface, the vtbl, and the
 * model.
 *
 * The physical interface is what sits on the stack - the handle to the rest of
 * the structure. It contains the pointer to the model, and optionally a small
 * buffer, where models that fit into it can reside. It is constructed through
 * the 'any_interface' constructor.
 *
 * The vtbl is the specification of the vtable - a contract that the model
 * implements. It is the abstract base class of the model, and specifies how the
 * physical interface communicates to the model through the model pointer. The
 * vtbl is never actually instantiated. It is constructed by the any_concept
 * constructor.
 *
 * The model is the implementation of the physical interface - the concrete
 * class. It is dynamically allocated and is either placed onto the heap or
 * into the small buffer that the interface holds as an optimization. It is
 * constructed with the any_model constructor.
 *
 * When writing new features, the idea is the following:
 * * the feature's interface is what the user of an `any` sees - operators,
 *   methods, even free functions. The parts that comprise the interface call
 *   implementations of those methods in the model.
 * * the vtbl declares the abstract interface for that part of the model.
 * * the model implements the dispatch to a part of the value.
 *
 * As a rule, you should not need more than a screenful of code (about 40
 * lines) to implement any given feature, most of which should not actually be
 * code that does anything.
 *
 * ### Composing interface, vtbl and model
 *
 * All three classes - interface, vtbl, and model - are composed of a number
 * of other classes that give them their features.
 *
 * Every feature provides its implementation of all three classes: its part of
 * the interface, vtbl and model. See equality_comparable for a basic
 * example, and callable for a slightly more advanced example.
 * value_equality_comparable gives a minimal example.
 *
 * For every feature, the feature's interface, vtbl and model are grouped
 * by a feature class (see examples from the previous paragraph). The feature
 * class provides the mechanism by which the feature's implementation classes
 * can be found by the construction mechanism.
 *
 * ### The feature class interface
 *
 * For examples, see implementations below. The interface that the construction
 * mechanism requires is:
 *
 *     struct my_feature_class { // the name will be used by the clients!
 *       template <typename BaseConcept>
 *       using vtbl = my_concept<BaseConcept>;       // vtbl struct
 *       template <typename BaseModel>
 *       using model = my_model<BaseModel>;             // model struct
 *       template <typename BaseInterface>
 *       using interface = my_interface<BaseInterface>; // interface struct
 *     };
 *
 * Each struct MUST, AND SHOULD ONLY inherit from its Base that is passed to it.
 * - 'must': we want to set up an empty-baseclass chain so that our objects are
 *   not large. Not inheriting breaks the chain, preventing all features after
 *   this struct from being included in the chain.
 * - 'should only': in multiple inheritance, different base classes must be
 *   given different addresses. Single inheritance breaks this rule. Inheriting
 *   from more than one class will cause your objects to be larger than
 *   possible.
 *
 * ### Base Class facilities
 *
 * To make the above classes easier to write, the library offers a number of
 * facilities in the BaseConcept, BaseModel and BaseInterface classes that are
 * passed to each object.
 *
 * All of these are in namespace `erasure::feature_support`.
 *
 * #### For Concepts ####
 *
 * Let C be the base vtbl, as here:
 *
 *     template <typename C>
 *     struct my_concept : C {
 *       // use the features in here, as shown
 *     };
 *
 * `vtbl<C>` is the full vtbl type. References and const references to
 * `vtbl<C>` should be taken by your interface functions. It is the full
 * 'name' of the abstract class that my_concept is part of.
 *
 * `m_storage<C>` is the storage type that the underlying model uses. You
 * probably won't need this one, since all that is taken care of for you by the
 * copy/move classes in the core library.
 *
 * #### For Models
 * Let M be the base model, as here:
 *
 *     template <typename M>
 *     struct my_model : M{
 *       // use the features in here, as shown
 *     };
 *
 * All the support given to concepts is given to models. In addition, the
 * following functions and type functions are provided:
 *
 * `m_model<C>` is the full model type, including value. See its usage in this
 * file.
 *
 * `m_value<C>` is the value type the model is holding. You would need that to
 * implement casts, but fortunately, that is provided for you.
 *
 * `M::self()` is the `*this` pointer, cast to the type of `m_model<M>`, and is
 * a reference to it. Use it instead of `*this` in the methods you write.
 *
 * `M::self_cast(vtbl<M> const& y)` casts a reference to a vtbl you
 * received as a function parameter to the type of the model. Use it for
 * methods that their interface only forwards their own type. Comes in a
 * non-const version too.
 *
 * #### For Interfaces ####
 *
 * The interfaces have a bit less support, since anything that the base
 * interface would provide ends up in the final class, and we don't want that.
 * Be mindful that your interface only uses and provides what it really needs
 * to. No extraneous typedefs outside of methods, and such. This is the chief
 * reason why all utilities for interfaces are provided as free functions.
 *
 * Because of this, the support for interfaces is provided a little less
 * elegantly.
 *
 * Let I be the base interface, as here:
 *
 *     template <typename I>
 *     struct my_interface : I {
 *       // use the features here, as shown
 *     };
 *
 * `erasure::ifc< I >` is the base any_t type.
 *
 * `ifc_concept< I >` is the vtbl type.
 *
 * `ifc_tags< I >` are all the feature tags that make up this any, except for
 * storage-related ones.
 *
 * `ifc_concept_ptr(*this)`, with `this` being an interface implementation,
 * gives you the model pointer of an `any`, with the type of
 * `ifc_concept< I * >&`. You will need this to dispatch the call from the
 * interface method to the implementation method in the model.  Use as
 * `ifc_concept_ptr(*this)->my_method(...)`. A sister method is
 * `concept_ptr(erasure::ifc< I >&)`, but you can only use that one with
 * correctly typed arguments.
 *
 * `ifc_self_cast(*this)`, with `this` being an interface implementation, gives
 * you the `self_any_type< I >`'d ref to self. You shouldn't need this often,
 * but if you do, it's available.
 */

// for the storage
#include "small_buffer.hpp"
// for all options interpretation
#include "meta.hpp"

#include <cassert>
#include <cstdint>
#include <new> // for placement new
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility> // for std::move and std::forward

namespace erasure {

/* *****************************************************************************
 * Convenience functions
 * ****************************************************************************/
/**
 * Tell if the pointees have the same dynamic type.
 * @param x the first pointer.
 * @param y the second pointer.
 * @return if *x and *y have the same dynamic type, return true, else false.
 *
 * Note: the empty typenames with default values are there so that they assert
 * the dereference operation exists for both types, and SFINAE out if it
 * doesn't.
 */
template <typename Pointer1, typename Pointer2,
          typename = decltype(*std::declval<Pointer1>()),
          typename = decltype(*std::declval<Pointer2>())>
auto same_dynamic_type(Pointer1 const &x, Pointer2 const &y) -> bool {
  // typeid's don't have to have the same address, even for the same type, if
  // they are from different compilation units. Even from the same compilation
  // unit, for some reason unknown to me, they sometimes don't compare equal.
  // Comparing type indices is correct (the standard requires it), and always
  // works.
  return x && y && (std::type_index(typeid(*x)) == std::type_index(typeid(*y)));
}
template <typename AnyOptions>
struct any_t;
template <typename Interface>
auto concept_ptr(Interface &&x) -> auto *;
template <typename Feature>
struct tag_t final {};

template <typename Feature>
inline constexpr tag_t<Feature> tag = {};

struct copy_assignable;
struct copy_constructible;
struct move_assignable;
struct move_constructible;

namespace detail {
template <typename Features>
struct feature_group {
  using features = Features;
  using provides_tag = typename meta::head_t<Features>::provides;
};

template <template <typename, typename> class Extract,
          template <template <typename> class, typename> class Compose,
          typename Features, typename Base>
struct compose_feature_group {
  template <typename Feature>
  struct curry_feature {
    template <typename Base_>
    using type = Extract<Feature, Base_>;
  };

  template <typename Feature1, typename Feature2>
  using compose = Compose<curry_feature<Feature1>::template type, Feature2>;

  using head = meta::head_t<Features>;
  using rest = meta::tail_t<Features>;
  using base = Extract<head, Base>;
  using type = meta::foldr_t<compose, rest, base>;
};
template <template <typename, typename> class Extract,
          template <template <typename> class, typename> class Compose,
          typename Features, typename Base>
using compose_feature_group_t =
    typename compose_feature_group<Extract, Compose, Features, Base>::type;

struct apply {
  template <template <typename> class T, typename U>
  using type = T<U>;
};
template <typename>
using compose_models_t = apply;

template <typename ProvidesDefinition>
struct compose_interfaces {
  template <template <typename> class T, typename U>
  using type = typename ProvidesDefinition::template importer<T, U>;
};
template <typename ProvidesDefinition>
using compose_interfaces_t = compose_interfaces<ProvidesDefinition>;

template <template <typename, typename> class Extract,
          template <typename> class Composer, typename FeatureGroups,
          typename Base>
struct compose_feature_groups {
  template <typename Group, typename InheritBase>
  using compose_group = compose_feature_group_t<
      Extract, Composer<typename Group::provides_tag>::template type,
      typename Group::features, InheritBase>;
  using type = meta::foldr_t<compose_group, FeatureGroups, Base>;
};
template <template <typename, typename> class Extract,
          template <typename> class Composer, typename FeatureGroups,
          typename Base>
using compose_feature_groups_t =
    typename compose_feature_groups<Extract, Composer, FeatureGroups,
                                    Base>::type;

/**
 * feature_concept_check - this will break if a tag isn't proper.
 */
template <typename FeatureClass>
struct feature_concept_check {
  template <typename B>
  using has_model = typename FeatureClass::template model<B>;
  template <typename B>
  using has_interface = typename FeatureClass::template interface<B>;
  template <typename B>
  using has_concept = typename FeatureClass::template vtbl<B>;
  using type = std::true_type;
};
template <typename T>
using feature_concept_check_t = typename feature_concept_check<T>::type;
} // namespace detail

namespace detail {
struct sizeof_alignof;
struct target_type;
struct allocate;
struct copy_construct_in;
struct allocate_and_copy_construct_in;
struct move_construct_in;
struct allocate_and_move_construct_in;

// user interface
template <typename ConceptBase>
struct concept_traits;
template <typename Concept, typename AnyOptions>
struct concept_base;

template <typename Concept, typename AnyOptions>
struct concept_traits<concept_base<Concept, AnyOptions>> {
  using concept_type = Concept;
  using pointer = concept_type *;
  using const_pointer = concept_type const *;
  using options = AnyOptions;
  using storage_type =
      ubuf::small_buffer<(typename options::buffer_actual_size){}>;
};

template <typename Concept, typename AnyOptions>
struct concept_base {
  // copy construction support
  virtual auto erase(tag_t<sizeof_alignof>) const -> ubuf::buffer_spec = 0;
  // type support
  virtual auto erase(tag_t<target_type>) const -> std::type_info const & = 0;
  /**
   * @param buf inout.
   */
  virtual auto
  erase(tag_t<allocate>,
        typename concept_traits<concept_base>::storage_type &buf) const
      -> ubuf::buffer_t = 0;

  /**
   * Make sure we always have a virtual destructor to call.
   */
  virtual ~concept_base() noexcept {};
};

template <typename Concept, typename AnyOptions>
constexpr auto
concept_base_type(concept_base<Concept, AnyOptions> const &) noexcept
    -> concept_base<Concept, AnyOptions> &;

template <typename T>
using concept_traits_t = concept_traits<std::remove_reference_t<decltype(
    detail::concept_base_type(std::declval<T &>()))>>;

} // namespace detail
template <typename BaseModel>
using vtbl = typename detail::concept_traits_t<BaseModel>::concept_type;
namespace detail {
template <typename BaseModel>
using m_storage = typename concept_traits_t<BaseModel>::storage_type;

template <typename Tag, typename Base>
using link_concepts = typename Tag::template vtbl<Base>;

/* *****************************************************************************
 * Type function: chain_concepts
 *
 * Constructs a CRTP-based single-inheritance stack from a parameter-pack of
 * tags.
 * @param Concept the final CRTP-class, which ends up being the model vtbl
 * type.
 * @param Tags tags that specify the vtbl to inherit from.
 * **************************************************/
template <typename Concept, typename AnyOptions>
using chain_concepts =
    compose_feature_groups_t<link_concepts, compose_models_t,
                             typename AnyOptions::feature_groups,
                             concept_base<Concept, AnyOptions>>;

/* ****************************************************************
 * type function: vtbl
 * ****************************************************************/
template <typename AnyOptions>
struct concept_t : chain_concepts<concept_t<AnyOptions>, AnyOptions> {};

template <typename AnyOptions>
using any_concept = concept_t<AnyOptions>;

template <typename Value, typename Model, typename Concept>
struct model_base;
template <typename>
struct model_traits;

template <typename Value, typename Model, typename Concept>
struct model_traits<model_base<Value, Model, Concept>> {
  using value_type = Value;
  using model_type = Model;
};

template <typename Value, typename Model, typename Concept>
constexpr auto
model_base_type(model_base<Value, Model, Concept> const &) noexcept
    -> model_base<Value, Model, Concept> &;

template <typename T>
using model_traits_t = model_traits<std::remove_reference_t<decltype(
    ::erasure::detail::model_base_type(std::declval<T &>()))>>;

template <typename BaseModel>
using m_value = typename model_traits_t<BaseModel>::value_type;
template <typename BaseModel>
using m_model = typename model_traits_t<BaseModel>::model_type;
} // namespace detail

inline namespace self_impl {
constexpr inline struct self_t final {
  template <typename Self>
  [[gnu::always_inline]] friend inline auto tag_invoke(erasure::self_t,
                                                       Self &&x) noexcept
      -> meta::copy_cvref_t<Self &&, detail::m_model<Self>> {
    return static_cast<meta::copy_cvref_t<Self &&, detail::m_model<Self>>>(x);
  }
  template <typename Self>
  [[gnu::always_inline]] inline auto operator()(Self &&model) const noexcept
      -> decltype(tag_invoke(std::declval<self_t>(), (Self &&) model)) {
    return tag_invoke(self_t{}, (Self &&) model);
  }
} self;
} // namespace self_impl
inline namespace value_impl {
constexpr inline struct value_t final {
  template <typename Self>
  [[gnu::always_inline]] friend inline auto tag_invoke(erasure::value_t,
                                                       Self &&x) noexcept
      -> meta::copy_cvref_t<Self &&, detail::m_value<Self>> {
    return erasure::self((Self &&) x)._value;
  }
  template <typename Self>
  [[gnu::always_inline]] inline auto operator()(Self &&model) const noexcept
      -> decltype(tag_invoke(std::declval<value_t>(), (Self &&) model)) {
    return tag_invoke(value_t{}, (Self &&) model);
  }
} value;
} // namespace value_impl
inline namespace self_cast_impl {
constexpr inline struct self_cast_t final {
  template <typename Self, typename Other>
  [[gnu::always_inline]] friend inline auto
  tag_invoke(erasure::self_cast_t, Self &&, Other &&y) noexcept
      -> meta::copy_cvref_t<Other &&, detail::m_model<Self>> {
    return static_cast<meta::copy_cvref_t<Other &&, detail::m_model<Self>>>(y);
  }

  template <typename Self, typename Other>
  [[gnu::always_inline]] inline auto operator()(Self &&model,
                                                Other &&other) const noexcept
      -> decltype(tag_invoke(std::declval<self_cast_t>(), (Self &&) model,
                             (Other &&) other)) {
    assert(erasure::same_dynamic_type(&model, &other) && "precondition");
    return tag_invoke(self_cast_t{}, (Self &&) model, (Other &&) other);
  }
} self_cast;
} // namespace self_cast_impl

namespace detail {
/**
 * In the end, provide a few useful typedefs and self(), and inherit from
 * Concept to get all of the stuff from it.
 */
template <typename Value, typename Model, typename Concept>
struct model_base : Concept {
  friend erasure::self_t;
  friend erasure::value_t;
  friend erasure::self_cast_t;

  // dynamic queries
  auto erase(tag_t<sizeof_alignof>) const -> ubuf::buffer_spec final {
    return {sizeof(m_model<model_base>), alignof(m_model<model_base>)};
  }
  auto erase(tag_t<target_type>) const -> std::type_info const & final {
    return typeid(m_value<model_base>);
  }
  auto erase(tag_t<allocate>, m_storage<Concept> &buf) const
      -> ubuf::buffer_t final {
    return buf.template allocate<m_model<model_base>>();
  }
};

template <typename Tag, typename Base>
using link_models = typename Tag::template model<Base>;

template <typename Value, typename Model, typename Concept>
using chain_models = compose_feature_groups_t<
    link_models, compose_models_t,
    typename concept_traits_t<Concept>::options::feature_groups,
    model_base<Value, Model, Concept>>;

/** SUPPORT FOR std::reference_wrapper */
template <typename Value>
struct reference_type {
  using value_type = Value;
  using type = Value;
  using reference = type &;
  using const_reference = type const &;
};

template <typename Value>
struct reference_type<std::reference_wrapper<Value>> {
  using value_type = std::reference_wrapper<Value>;
  using type = Value;
  using reference = type &;
  using const_reference = type const &;
};

/* ******************************************************************
 * type function: model
 * ******************************************************************/
template <typename Value, typename Concept>
struct model_t : chain_models<Value, model_t<Value, Concept>, Concept> {
  friend erasure::value_t;
  // repeat base type...
  static_assert(std::is_same<m_value<model_t>, Value>(),
                "chain_models bug: incorrect chaining of value_type.");

  model_t() {}
  model_t(Value &&x) : _value(std::move(x)) {}
  model_t(Value const &x) : _value(x) {}

  Value _value;
};

template <typename Value, typename Concept>
using any_model = model_t<Value, Concept>;

/* ******************************************************************
 * type function: chain_interfaces
 * ******************************************************************/

template <typename Tag, typename Base>
using link_interfaces = typename Tag::template interface<Base>;
template <typename Tag, typename Base1, typename Base2>
using link_importers = typename Tag::template importer<Base1, Base2>;

// base of recursion
template <typename InterfaceTraits>
struct interface_base {};

template <typename InterfaceTraits>
constexpr auto interface_support_type(interface_base<InterfaceTraits> const &)
    -> InterfaceTraits;

// prevent clashes
template <typename T>
using interface_traits_t =
    decltype(::erasure::detail::interface_support_type(std::declval<T>()));

template <typename T>
using ifc_interface = typename interface_traits_t<T>::interface_type;
template <typename T, typename V>
using ifc_model = typename interface_traits_t<T>::template model_type<V>;
template <typename T>
using ifc_concept = typename interface_traits_t<T>::concept_type;
template <typename T>
using ifc_tags = typename interface_traits_t<T>::tags;
} // namespace detail
template <typename T>
using ifc = typename detail::interface_traits_t<T>::any_type;

namespace detail {
template <typename T>
decltype(auto) ifc_self_cast(T &&x) {
  return meta::forward_cast<erasure::ifc<T>>((T &&) x);
}
template <typename T>
auto ifc_concept_ptr(T &&x) {
  return erasure::concept_ptr(detail::ifc_self_cast((T &&) x));
}

template <typename Interface, typename InterfaceTraits>
using chain_interfaces =
    compose_feature_groups_t<link_interfaces, compose_interfaces_t,
                             typename InterfaceTraits::options::feature_groups,
                             interface_base<InterfaceTraits>>;
/* *****************************************************************
 * type function: interface_t
 * *****************************************************************/

// forward declare interface_t because its support class needs it.
template <bool, bool, bool, bool, typename>
struct interface_t;

template <typename Concept, template <typename> class Model, typename AnyType>
struct interface_traits {
  using options = typename concept_traits_t<Concept>::options;
  using tags = typename options::tags;
  static_assert(meta::is_typelist<tags>{}, "Taglist needs to be a typelist!");
  static_assert(meta::all_t<feature_concept_check_t, tags>{},
                "All members must be features.");

  using is_move_constructible = meta::is_element_t<move_constructible, tags>;
  using is_move_assignable = meta::is_element_t<move_assignable, tags>;
  using is_copy_constructible = meta::is_element_t<copy_constructible, tags>;
  using is_copy_assignable = meta::is_element_t<copy_assignable, tags>;

  using any_type = AnyType;

  template <typename T>
  using is_self_type = std::is_same<std::decay_t<T>, any_type>;

  using concept_type = Concept;

  template <typename V>
  using model_type = Model<V>;
  // the incomplete type of the interface. Is what is used in the interface
  // chain.
  using interface_incomplete =
      interface_t<is_move_constructible{}, is_move_assignable{},
                  is_copy_constructible{}, is_copy_assignable{},
                  interface_traits>;
  using interface_type = interface_incomplete;

  // the complete type of the interface. A problem if used in certain contexts
  using interface_complete =
      chain_interfaces<interface_incomplete, interface_traits>;
  // what any inherits from
  using base = interface_complete;
  using storage = m_storage<concept_type>;
};
template <typename Support, typename Any1>
auto self_any_cast(Any1 &&x) -> decltype(auto) {
  return meta::forward_cast<typename Support::any_type>((Any1 &&) x);
}

/* forward-declare functions we need to implement interface_t. */
template <typename Interface>
void reset(Interface &x);
template <typename Interface>
auto buffer_ref(Interface &&x) -> decltype(auto);
} // namespace detail

template <typename Tag, typename Interface, typename... As>
[[gnu::always_inline]] inline auto call(Interface &&x, As &&... as)
    -> decltype(auto) {
  return ifc_concept_ptr(x)->erase(tag<Tag>, (As &&) as...);
}
namespace detail {
template <typename AnyOptions, typename T>
void create_any_from_value(any_t<AnyOptions> &x, T &&value);

template <typename Support, typename T>
using disable_if_same_any_type =
    std::enable_if_t<!Support::template is_self_type<T>::value>;
template <typename S>
struct creation_support : S::base {
  creation_support() = default;
  template <typename T, typename = disable_if_same_any_type<S, T>>
  creation_support(T &&value) {
    auto &any_this = static_cast<typename S::any_type &>(*this);
    create_any_from_value(any_this, std::forward<T>(value));
  }
  template <typename T, typename = disable_if_same_any_type<S, T>>
  auto operator=(T &&value) -> typename S::any_type & {
    auto &any_this = static_cast<typename S::any_type &>(*this);
    reset(any_this);
    create_any_from_value(any_this, std::forward<T>(value));
    return any_this;
  }
  ~creation_support() { reset(self_any_cast<S>(*this)); }

private:
  friend struct interface_t_access;
  typename S::storage _any_ifc_value;
};

// MOVE IMPLEMENTATIONS
template <typename AO>
auto move_construct_any(any_t<AO> &target, any_t<AO> &&source) -> any_t<AO> & {
  if (buffer_ref(source)) {
    erasure::call<allocate_and_move_construct_in>(source, buffer_ref(target));
  }
  return target;
}
template <typename AO>
auto move_assign_any(any_t<AO> &target, any_t<AO> &&source,
                     /* is_move_assignable */ std::false_type) -> any_t<AO> & {
  reset(target);
  move_construct_any(target, std::move(source));
  return target;
}
template <typename AO>
auto move_assign_any(any_t<AO> &target, any_t<AO> &&source,
                     /* is_move_assignable */ std::true_type) -> any_t<AO> & {
  if (same_dynamic_type(target, source)) {
    erasure::call<move_assignable>(target,
                                   std::move(*erasure::concept_ptr(source)));
  } else {
    move_assign_any(target, std::move(source), std::false_type{});
  }
  return target;
}

// COPY IMPLEMENTATIONS
template <typename Any1, typename Any2>
void copy_construct_any(Any1 &target, Any2 const &source) {
  if (buffer_ref(source)) {
    erasure::call<allocate_and_copy_construct_in>(source, buffer_ref(target));
  }
}
/** For copy_assign_any, the last parameter is is_copyable */
template <typename Any1, typename Any2>
auto copy_assign_any(Any1 &target, Any2 const &source, std::false_type)
    -> Any1 & {
  reset(target);
  copy_construct_any(target, source);
  return target;
}
template <typename Any1, typename Any2>
auto copy_assign_any(Any1 &target, Any2 const &source, std::true_type)
    -> Any1 & {
  if (same_dynamic_type(target, source)) {
    erasure::call<copy_assignable>(target, *erasure::concept_ptr(source));
  } else {
    copy_assign_any(target, source, std::false_type{});
  }
  return target;
}

/*** INTERFACE_T INSTANTIATIONS ***/

#define INTERFACE_T_MOVE_CONSTRUCTOR                                           \
  interface_t(interface_t &&x) {                                               \
    move_construct_any(self_any_cast<S>(*this),                                \
                       std::move(self_any_cast<S>(x)));                        \
  }                                                                            \
  static_assert(true, "")

#define INTERFACE_T_COPY_CONSTRUCTOR                                           \
  interface_t(interface_t const &x) {                                          \
    copy_construct_any(self_any_cast<S>(*this), self_any_cast<S>(x));          \
  }                                                                            \
  static_assert(true, "")

#define INTERFACE_T_MOVE_ASSIGNMENT                                            \
  interface_t &operator=(interface_t &&x) {                                    \
    return move_assign_any(self_any_cast<S>(*this),                            \
                           std::move(self_any_cast<S>(x)),                     \
                           typename S::is_move_assignable{});                  \
  }                                                                            \
  static_assert(true, "")

#define INTERFACE_T_COPY_ASSIGNMENT                                            \
  interface_t &operator=(interface_t const &x) {                               \
    return copy_assign_any(self_any_cast<S>(*this), self_any_cast<S>(x),       \
                           typename S::is_copy_assignable{});                  \
  }                                                                            \
  static_assert(true, "")

/**
 * Dispatch mechanism for copy / move construction.
 * Unfortunately, we cannot simply inherit these easily. We have to dispatch to
 * the class that correctly deletes / preserves the appropriate constructors.
 */
template <typename S>
struct interface_t<true, true, true, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  INTERFACE_T_MOVE_ASSIGNMENT;
  INTERFACE_T_COPY_CONSTRUCTOR;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<true, true, true, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  INTERFACE_T_MOVE_ASSIGNMENT;
  INTERFACE_T_COPY_CONSTRUCTOR;
  auto operator=(interface_t const &x) -> interface_t & = delete;
};
template <typename S>
struct interface_t<true, true, false, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  INTERFACE_T_MOVE_ASSIGNMENT;
  interface_t(interface_t const &x) = delete;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<true, true, false, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  INTERFACE_T_MOVE_ASSIGNMENT;
  interface_t(interface_t const &x) = delete;
  auto operator=(interface_t const &x) -> interface_t & = delete;
};
template <typename S>
struct interface_t<true, false, true, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  auto operator=(interface_t &&x) -> interface_t & = delete;
  INTERFACE_T_COPY_CONSTRUCTOR;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<true, false, true, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  auto operator=(interface_t &&x) -> interface_t & = delete;
  INTERFACE_T_COPY_CONSTRUCTOR;
  auto operator=(interface_t const &x) -> interface_t & = delete;
};
template <typename S>
struct interface_t<true, false, false, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  auto operator=(interface_t &&x) -> interface_t & = delete;
  interface_t(interface_t const &x) = delete;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<true, false, false, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  auto operator=(interface_t &&x) -> interface_t & = delete;
  interface_t(interface_t const &x) = delete;
  auto operator=(interface_t const &x) -> interface_t & = delete;
};
template <typename S>
struct interface_t<false, true, true, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t &&x) = delete;
  INTERFACE_T_MOVE_ASSIGNMENT;
  INTERFACE_T_COPY_CONSTRUCTOR;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<false, true, true, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t &&x) = delete;
  INTERFACE_T_MOVE_ASSIGNMENT;
  INTERFACE_T_COPY_CONSTRUCTOR;
  auto operator=(interface_t const &x) -> interface_t & = delete;
};
template <typename S>
struct interface_t<false, true, false, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t &&x) = delete;
  INTERFACE_T_MOVE_ASSIGNMENT;
  interface_t(interface_t const &x) = delete;
  INTERFACE_T_COPY_ASSIGNMENT;
  static_assert(!std::is_same<S, S>{},
                "Types that are not either move or copy "
                "constructible are not supported.");
};
template <typename S>
struct interface_t<false, true, false, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t &&x) = delete;
  INTERFACE_T_MOVE_ASSIGNMENT;
  interface_t(interface_t const &x) = delete;
  auto operator=(interface_t const &x) -> interface_t & = delete;
  static_assert(!std::is_same<S, S>{},
                "Types that are not either move or copy "
                "constructible are not supported.");
};
template <typename S>
struct interface_t<false, false, true, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t &&x) = delete;
  auto operator=(interface_t &&x) -> interface_t & = delete;
  INTERFACE_T_COPY_CONSTRUCTOR;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<false, false, true, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t &&x) = delete;
  auto operator=(interface_t &&x) -> interface_t & = delete;
  INTERFACE_T_COPY_CONSTRUCTOR;
  auto operator=(interface_t const &x) -> interface_t & = delete;
};
template <typename S>
struct interface_t<false, false, false, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t &&x) = delete;
  auto operator=(interface_t &&x) -> interface_t & = delete;
  interface_t(interface_t const &x) = delete;
  INTERFACE_T_COPY_ASSIGNMENT;
  static_assert(!std::is_same<S, S>{},
                "Types that are not either move or copy "
                "constructible are not supported.");
};
template <typename S>
struct interface_t<false, false, false, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t &&x) = delete;
  auto operator=(interface_t &&x) -> interface_t & = delete;
  interface_t(interface_t const &x) = delete;
  auto operator=(interface_t const &x) -> interface_t & = delete;
  static_assert(!std::is_same<S, S>{},
                "Types that are not either move or copy "
                "constructible are not supported.");
};

#undef INTERFACE_T_COPY_ASSIGNMENT
#undef INTERFACE_T_MOVE_ASSIGNMENT
#undef INTERFACE_T_COPY_CONSTRUCTOR
#undef INTERFACE_T_MOVE_CONSTRUCTOR

struct interface_t_access {
  template <typename S>
  static constexpr auto get_creation_support(creation_support<S> &x)
      -> decltype(auto) {
    return x;
  }
  template <typename S>
  static constexpr auto get_creation_support(creation_support<S> const &x)
      -> decltype(auto) {
    return x;
  }
  template <bool MC, bool MA, bool CC, bool CA, typename S>
  static auto value_ptr(interface_t<MC, MA, CC, CA, S> &x) -> auto & {
    return get_creation_support(x)._any_ifc_value;
  }
  template <bool MC, bool MA, bool CC, bool CA, typename S>
  static auto value_ptr(interface_t<MC, MA, CC, CA, S> const &x)
      -> auto const & {
    return get_creation_support(x)._any_ifc_value;
  }
};
template <typename Interface>
auto buffer_ref(Interface &&x) -> decltype(auto) {
  return interface_t_access::value_ptr(x);
}
} // namespace detail
template <typename Interface>
auto concept_ptr(Interface &&x) -> auto * {
  return static_cast<
      meta::copy_const_t<Interface, detail::ifc_concept<Interface>> *>(
      detail::buffer_ref(x).get());
}
namespace detail {
template <typename T, typename Interface>
auto model_ptr(Interface &&x) -> auto * {
  auto cptr = erasure::concept_ptr((Interface &&) x);
  auto const impl_p = dynamic_cast<ifc_model<Interface &&, T> const *>(cptr);
  return impl_p ? impl_p : nullptr;
}

template <typename Interface>
void reset(Interface &x) {
  using vtbl = ifc_concept<Interface>;
  auto value = erasure::concept_ptr(x);
  if (value) {
    value->~vtbl();
    buffer_ref(x).reset();
  }
}
} // namespace detail

template <typename AO>
auto same_dynamic_type(any_t<AO> const &x, any_t<AO> const &y) -> bool {
  return same_dynamic_type(erasure::concept_ptr(x), erasure::concept_ptr(y));
}

namespace detail {
/**
 * The any_interface constructor.
 * Calculates the support type for the interface, and the vtbl and model
 * types for it.
 */
template <typename AnyOptions>
struct any_interface_t {
  using tags = typename AnyOptions::tags;
  using vtbl = any_concept<AnyOptions>;
  template <typename T>
  using model = any_model<std::remove_cvref_t<T>, vtbl>;
  using support_type = interface_traits<vtbl, model, any_t<AnyOptions>>;
  using type = typename support_type::interface_type;
};
template <typename AnyOptions>
using any_interface = typename any_interface_t<AnyOptions>::type;

template <typename Any>
struct get_options_t;
template <typename AnyOptions>
struct get_options_t<any_t<AnyOptions>> {
  using type = AnyOptions;
};
template <typename Any>
using get_options = typename get_options_t<Any>::type;

/* *********************************************************************
 * ANY OPTION PROCESSING
 * *********************************************************************/

/* *********************************************************************
 * OPTIONS
 * *********************************************************************/
using meta::copy_if_t;
using meta::typelist;
} // namespace detail

/** The option for the inner buffer size. */
template <std::size_t BufferSize>
struct buffer_size : std::integral_constant<std::size_t, BufferSize> {};
namespace detail {
template <std::size_t Size>
struct feature_concept_check<buffer_size<Size>> {
  using type = std::true_type;
};
/** The predicate that tells us whether a tag is a buffer_size */
template <typename T>
struct is_buffer_size : std::false_type {};
template <std::size_t BufferSize>
struct is_buffer_size<buffer_size<BufferSize>> : std::true_type {};

using meta::head_t;

using meta::and_;
using meta::concatenate_t;
using meta::copy_if_not_t;
using meta::find_first_t;
using meta::foldl_t;
using meta::map_t;

template <typename Taglist>
struct any_options {
  using all_tags = Taglist;
  using all_tags_default_size =
      concatenate_t<all_tags, typelist<buffer_size<0>>>;
  using tags = copy_if_not_t<is_buffer_size, all_tags>;
  using buffer_actual_size =
      find_first_t<is_buffer_size, all_tags_default_size>;

  template <typename F1, typename F2>
  using equal_provides =
      meta::is_same_t<typename F1::provides, typename F2::provides>;

  using feature_groups =
      map_t<feature_group, meta::group_by_t<equal_provides, tags>>;
};

/**
 * Should be the same as is_same, except compare buffer_size features without
 * regard for their parameter.
 */
template <typename T, typename U>
using tag_equivalence =
    std::conditional_t<std::is_same<T, U>{}, std::true_type,
                       and_<is_buffer_size<T>, is_buffer_size<U>>>;
/**
 * Flatten the options list and deduplicate tags. Take the earliest of any tag
 * encountered and forget all subsequent ones. Treat buffer_size parameters as
 * equivalent and only take the first one.
 */
template <typename Typelist>
using make_options =
    any_options<meta::unique_t<meta::flatten_t<Typelist>, tag_equivalence>>;
template <typename... Tags>
using options = make_options<typelist<Tags...>>;

/* the is_any type-trait */
template <typename T>
struct is_any_t : std::false_type {};
template <typename AnyOptions>
struct is_any_t<any_t<AnyOptions>> : std::true_type {};
template <typename T>
using is_any = typename is_any_t<T>::type;

template <typename... Features>
struct make_any_t {
  using opts = options<Features...>;
  // check for common errors:
  // - passing any_options as tags
  template <typename AnyOptions>
  struct is_any_options : std::false_type {};
  template <typename Taglist>
  struct is_any_options<any_options<Taglist>> : std::true_type {};
  static_assert(!meta::any_t<is_any_options, typelist<Features...>>{},
                "Please remove the any_options object from the Features.");
  // - passing an any as tags
  using type = any_t<opts>;
};

} // namespace detail
template <typename AnyOptions>
struct any_t : detail::any_interface<AnyOptions> {
  using _any_base = detail::any_interface<AnyOptions>;
  // inherit constructors
  using _any_base::_any_base;
  using _any_base::operator=;

  friend inline auto empty(any_t const &x) -> bool {
    return concept_ptr(x) == nullptr;
  }
};
template <typename... Features>
using any = typename detail::make_any_t<Features...>::type;

namespace detail {
/**
 * Construct a model in the storage buffer provided.
 *
 * The model pointer returned points to storage.data. The model should be
 * destroyed manually by calling its destructor.
 *
 * @param storage a bunch of untyped memory
 * @param x the value to move-construct in the model.
 * @param Model the model type.
 */
template <typename Model, typename T, typename U>
auto make_model(ubuf::buffer_t storage, T &&x, std::true_type, U)
    -> vtbl<Model> * {
  using std::forward;
  using model = Model;

  assert(reinterpret_cast<std::intptr_t>(storage.data) % alignof(model) == 0);
  assert(sizeof(model) <= storage.size);

  return new (storage.data) model{forward<T>(x)};
}
template <typename Model, typename T>
auto make_model(ubuf::buffer_t storage, T &&x, std::false_type, std::true_type)
    -> vtbl<Model> * {
  using model = Model;

  assert(reinterpret_cast<std::intptr_t>(storage.data) % alignof(model) == 0);
  assert(sizeof(model) <= storage.size);

  auto c_ptr = new (storage.data) model{};
  c_ptr->value() = std::forward<T>(x);
  return c_ptr;
}
template <typename Model, typename T>
auto make_model(ubuf::buffer_t storage, T &&x, std::false_type, std::false_type)
    -> vtbl<Model> * {
  static_assert(!std::is_same<Model, Model>{},
                "To be move-assignable, a type needs to either "
                "be move constructible or "
                "default constructible.");
}
template <typename Model, typename T>
auto make_model(ubuf::buffer_t storage, T &&x) -> vtbl<Model> * {
  return make_model<Model>(storage, std::forward<T>(x),
                           std::is_move_constructible<T>{},
                           std::is_default_constructible<T>{});
}

template <typename AnyOptions, typename T>
void create_any_from_value(any_t<AnyOptions> &x, T &&value) {
  assert(empty(x));
  using model = ifc_model<decltype(x), T>;
  make_model<model>(buffer_ref(x).template allocate<model>(),
                    std::forward<T>(value));
}
} // namespace detail
template <typename... Tags, typename T>
auto make_any(T &&x) -> any<Tags...> {
  return {std::forward<T>(x)};
}

template <typename T, typename AnyOptions>
auto target(any_t<AnyOptions> const &x) -> T const * {
  using cast_to = detail::ifc_model<decltype(x), T>;
  auto const impl_p = dynamic_cast<cast_to const *>(concept_ptr(x));
  return impl_p ? &erasure::value(*impl_p) : nullptr;
}
template <typename T, typename AnyOptions>
auto target(any_t<AnyOptions> &x) -> T * {
  auto const &const_x = x;
  return const_cast<T *>(target<T const>(const_x));
}
template <typename T, typename AnyOptions>
auto target_type(any_t<AnyOptions> const &x) -> std::type_info const & {
  auto const pc = concept_ptr(x);
  return pc ? pc->get_target_type() : typeid(void);
}

/* feature definition helper */
struct feature {
  /**
   * A feature combiner without a "using I::myfunc;" declaration.
   *
   * See callable or dereferenceable for examples of combiners.
   */
  struct empty_provides_tag {
    template <template <typename> class T, typename U>
    struct importer : T<U> {};
  };
  using provides = empty_provides_tag;
};

/* ***********************************************************************
 * FEATURE DEFINITIONS
 * ***********************************************************************/

/* ***********************************************************
 * MOVE_CONSTRUCTIBLE
 * ***********************************************************/
struct move_constructible : feature {
  template <typename C>
  struct vtbl : C {
    using C::erase;
    virtual void erase(tag_t<detail::move_construct_in>,
                       ubuf::buffer_t buf) = 0;
    virtual void erase(tag_t<detail::allocate_and_move_construct_in>,
                       detail::m_storage<C> &buf) = 0;
  };

  template <typename M>
  struct model : M {
    using M::erase;
    void erase(tag_t<detail::move_construct_in>, ubuf::buffer_t buf) final {
      detail::make_model<detail::m_model<M>>(buf,
                                             erasure::value(std::move(*this)));
    }
    void erase(tag_t<detail::allocate_and_move_construct_in>,
               detail::m_storage<M> &buf) final {
      erase(tag<detail::move_construct_in>, erase(tag<detail::allocate>, buf));
    }
  };

  template <typename I>
  using interface = I;
};

/* ***********************************************************
 * MOVE ASSIGNABLE
 * ***********************************************************/
struct move_assignable : feature {
  template <typename C>
  struct vtbl : C {
    using C::erase;
    virtual void erase(tag_t<move_assignable>, erasure::vtbl<C> &&) = 0;
  };

  template <typename M>
  struct model : M {
    using M::erase;
    void erase(tag_t<move_assignable>, erasure::vtbl<M> &&y) final {
      erasure::value(*this) =
          erasure::value(erasure::self_cast(*this, std::move(y)));
    }
  };

  template <typename I>
  using interface = I;
};

/* ***********************************************************
 * COPYABLE
 * ***********************************************************/
struct copy_constructible : feature {
  template <typename C>
  struct vtbl : C {
    using C::erase;
    virtual void erase(tag_t<detail::copy_construct_in>,
                       ubuf::buffer_t) const = 0;
    virtual void erase(tag_t<detail::allocate_and_copy_construct_in>,
                       detail::m_storage<C> &) const = 0;
  };

  // model
  template <typename M>
  struct model : M {
    using M::erase;
    void erase(tag_t<detail::copy_construct_in>,
               ubuf::buffer_t buffer) const final {
      detail::make_model<detail::m_model<M>>(buffer, erasure::value(*this));
    }
    void erase(tag_t<detail::allocate_and_copy_construct_in>,
               detail::m_storage<M> &buf) const final {
      erase(tag<detail::copy_construct_in>, erase(tag<detail::allocate>, buf));
    }
  };

  template <typename I>
  using interface = I;
};

/* ***********************************************************
 * COPY_ASSIGNABLE
 * ***********************************************************/
struct copy_assignable : feature {
  template <typename C>
  struct vtbl : C {
    using C::erase;
    virtual void erase(tag_t<copy_assignable>, erasure::vtbl<C> const &x) = 0;
  };

  template <typename M>
  struct model : M {
    using M::erase;
    // precondition: y is of model_type.
    void erase(tag_t<copy_assignable>, erasure::vtbl<M> const &y) final {
      erasure::value(*this) = erasure::value(erasure::self_cast(*this, y));
    }
  };

  template <typename I>
  using interface = I;
};

/* ***************************************************************
 * SWAPPABLE
 * ***************************************************************/
struct swappable : feature {
  template <typename C>
  struct vtbl : C {
    using C::erase;
    virtual void erase(tag_t<swappable>, erasure::vtbl<C> &y) noexcept = 0;
  };

  template <typename M>
  struct model : M {
    using M::erase;
    // precondition: same type, ensured by swappable_interface
    void erase(tag_t<swappable>, erasure::vtbl<M> &y) noexcept final {
      using std::swap;
      swap(erasure::value(*this), erasure::value(erasure::self_cast(*this, y)));
    }
  };

  template <typename I>
  struct interface : I {
    friend void swap(erasure::ifc<I> &x, erasure::ifc<I> &y) noexcept {
      if (same_dynamic_type(x, y)) {
        erasure::call<swappable>(x, *erasure::concept_ptr(y));
      } else {
        // just swap the pointers
        if (!swap_if_not_internal(buffer_ref(x), buffer_ref(y))) {
          // if not swapped we need to go the slow way, using moves. Use
          // std::swap explicitly to not recurse into this function.
          std::swap(x, y);
        }
      }
    }
  };
};

template <typename AnyType, typename T>
auto make_any_like(T &&x) {
  return make_any<typename detail::get_options<AnyType>::all_tags>(
      std::forward<T>(x));
}
using meta::typelist;
using movable = typelist<move_constructible, move_assignable>;
using copyable = typelist<copy_constructible, copy_assignable>;

namespace debug {
template <typename Options>
auto model_size(any_t<Options> const &x) -> std::size_t {
  auto const pc = erasure::concept_ptr(x);
  return pc ? erasure::call<detail::sizeof_alignof>(x).size : 0;
}
} // namespace debug

// ################# END IMPLEMENTATION, BEGIN PUBLIC INTERFACE SYNOPSIS #######

namespace feature_support {
using erasure::any;
using erasure::call;
using erasure::concept_ptr;
using erasure::feature;
using erasure::ifc;
using erasure::make_any;
using erasure::make_any_like;
using erasure::same_dynamic_type;
using erasure::self;
using erasure::self_cast;
using erasure::tag;
using erasure::tag_t;
using erasure::target;
using erasure::target_type;
using erasure::value;
using erasure::vtbl;
using meta::typelist;
} // namespace feature_support

namespace features {
// type tags implementation
using erasure::buffer_size;
using erasure::copy_assignable;
using erasure::copy_constructible;
using erasure::move_assignable;
using erasure::move_constructible;
using erasure::swappable;
// type tag sets implementation
using erasure::copyable;
using erasure::movable;
} // namespace features

} /* namespace erasure */
