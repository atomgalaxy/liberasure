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
 * An any_t comprises three parts: the physical interface, the concept, and the
 * model.
 *
 * The physical interface is what sits on the stack - the handle to the rest of
 * the structure. It contains the pointer to the model, and optionally a small
 * buffer, where models that fit into it can reside. It is constructed through
 * the 'any_interface' constructor.
 *
 * The concept is the specification of the vtable - a contract that the model
 * implements. It is the abstract base class of the model, and specifies how the
 * physical interface communicates to the model through the model pointer. The
 * concept is never actually instantiated. It is constructed by the any_concept
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
 * * the concept declares the abstract interface for that part of the model.
 * * the model implements the dispatch to a part of the value.
 *
 * As a rule, you should not need more than a screenful of code (about 40
 * lines) to implement any given feature, most of which should not actually be
 * code that does anything.
 *
 * ### Composing interface, concept and model
 *
 * All three classes - interface, concept, and model - are composed of a number
 * of other classes that give them their features.
 *
 * Every feature provides its implementation of all three classes: its part of
 * the interface, concept and model. See equality_comparable for a basic
 * example, and callable for a slightly more advanced example.
 * value_equality_comparable gives a minimal example.
 *
 * For every feature, the feature's interface, concept and model are grouped
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
 *       using concept = my_concept<BaseConcept>;       // concept struct
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
 * Let C be the base concept, as here:
 * 
 *     template <typename C>
 *     struct my_concept : C {
 *       // use the features in here, as shown
 *     };
 *
 * `m_concept<C>` is the full concept type. References and const references to
 * `m_concept<C>` should be taken by your interface functions. It is the full
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
 * `M::self_cast(m_concept<M> const& y)` casts a reference to a concept you
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
 * `ifc_model< I, ValueType>` is the model type that holds this particular
 * value type.
 *
 * `ifc_interface< I >` is the full interface type.
 *
 * `ifc_any_type< I >` is the base any_t type.
 *
 * `ifc_concept< I >` is the concept type.
 *
 * `ifc_tags< I >` are all the feature tags that make up this any, except for
 * storage-related ones.
 *
 * `ifc_concept_ptr(*this)`, with `this` being an interface implementation,
 * gives you the model pointer of an `any`, with the type of
 * `ifc_concept< I * >&`. You will need this to dispatch the call from the
 * interface method to the implementation method in the model.  Use as
 * `ifc_concept_ptr(*this)->my_method(...)`. A sister method is
 * `concept_ptr(ifc_any_type< I >&)`, but you can only use that one with
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

namespace _1 {

/* *****************************************************************************
 * memory management types
 * ****************************************************************************/
using ubuf::owner;
using ubuf::buffer_t;
using ubuf::buffer_spec;

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
template <typename Pointer1,
          typename Pointer2,
          typename = decltype(*std::declval<Pointer1>()),
          typename = decltype(*std::declval<Pointer2>())>
bool same_dynamic_type(Pointer1 const& x, Pointer2 const& y) {
  // typeid's don't have to have the same address, even for the same type, if
  // they are from different compilation units. Even from the same compilation
  // unit, for some reason unknown to me, they sometimes don't compare equal.
  // Comparing type indices is correct (the standard requires it), and always
  // works.
  return x && y && (std::type_index(typeid(*x)) == std::type_index(typeid(*y)));
}

/**
 * The concept-checking class for the chainable concept.
 *
 * @param X is supposed to be a feature class.
 * @param Base is the base class that this chainable concept checks
 * @param Link is the binary type-function that will take X and Base and return
 * the subclass of the interface that this particular concept is checking for
 * 'chainable'.
 *
 * Chainable basically means that the class inherits from the base it's given,
 * and can be instantiated with that base.
 */
template <template <typename, typename> class Link, typename Base, typename X>
struct chainable_concept_check {
  using typealias = Link<X, Base>;
  static_assert(std::is_base_of<Base, typealias>{},
                "Your class does not inherit from its base.");
  typealias x;
};

template <typename Typelist>
struct multiply_inherit_all {
  static_assert(!std::is_same<Typelist, Typelist>{},
                "Needs a typelist to make sense.");
};
template <typename... Bases>
struct multiply_inherit_all<meta::typelist<Bases...>> : Bases... {};

template <template <typename, typename> class Link,
          typename Features,
          typename ChainBase>
struct chain {
  template <typename X>
  using assert_concept = chainable_concept_check<Link, ChainBase, X>;
  using assert_concepts = meta::map_t<assert_concept, Features>;
  using type = meta::foldr_t<Link, Features, ChainBase>;
};
template <template <typename, typename> class Link,
          typename Features,
          typename ChainBase>
using chain_t = typename chain<Link, Features, ChainBase>::type;

template <template <typename, typename> class Link,
          typename Features,
          typename ChainBase>
struct multiply_inherit {
  template <typename X>
  using assert_concept = chainable_concept_check<Link, ChainBase, X>;
  using assert_concepts = meta::map_t<assert_concept, Features>;
  using type = multiply_inherit_all<meta::map_t<Link, Features, ChainBase>>;
};
template <template <typename, typename> class Link,
          typename Features,
          typename ChainBase>
using multiply_inherit_t = typename chain<Link, Features, ChainBase>::type;

template <typename Features>
struct feature_group {
  using features = Features;
  using provides_tag = typename meta::head_t<Features>::provides;
};

template <template <typename, typename> class Extract,
          template <template <typename> class, typename> class Compose,
          typename Features,
          typename Base>
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
          typename Features,
          typename Base>
using compose_feature_group_t =
    typename compose_feature_group<Extract, Compose, Features, Base>::type;

template <typename ProvidesDefinition>
struct compose_models {
  template <template <typename> class T, typename U>
  using type = typename ProvidesDefinition::template model_name_unhider<T, U>;
};
template <typename ProvidesDefinition>
using compose_models_t = compose_models<ProvidesDefinition>;

template <typename ProvidesDefinition>
struct compose_interfaces {
  template <template <typename> class T, typename U>
  using type =
      typename ProvidesDefinition::template interface_name_unhider<T, U>;
};
template <typename ProvidesDefinition>
using compose_interfaces_t = compose_interfaces<ProvidesDefinition>;

template <template <typename, typename> class Extract,
          template <typename> class Composer,
          typename FeatureGroups,
          typename Base>
struct compose_feature_groups {
  template <typename Group, typename InheritBase>
  using compose_group = compose_feature_group_t<
      Extract,
      Composer<typename Group::provides_tag>::template type,
      typename Group::features,
      InheritBase>;
  using type = meta::foldr_t<compose_group, FeatureGroups, Base>;
};
template <template <typename, typename> class Extract,
          template <typename> class Composer,
          typename FeatureGroups,
          typename Base>
using compose_feature_groups_t =
    typename compose_feature_groups<Extract, Composer, FeatureGroups, Base>::
        type;

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
  using has_concept = typename FeatureClass::template concept<B>;
  using type = std::true_type;
};
template <typename T>
using feature_concept_check_t = typename feature_concept_check<T>::type;

/* *****************************************************************************
 * Type function: chain_concepts
 *
 * Constructs a CRPT-based single-inheritance stack from a parameter-pack of
 * tags.
 * @param Concept the final CRTP-class, which ends up being the model concept
 * type.
 * @param Tags tags that specify the concept to inherit from.
 * **************************************************/
template <typename Concept, typename AnyOptions>
struct concept_base {
  using concept_type = Concept;
  using pointer = concept_type*;
  using const_pointer = concept_type const*;
  using options = AnyOptions;
  using storage_type =
      ubuf::small_buffer<(typename options::buffer_actual_size){}>;

  // copy construction support
  virtual buffer_spec sizeof_alignof() const = 0;
  // type support
  virtual std::type_info const& target_type() const = 0;
  /**
   * @param buf inout.
   */
  virtual buffer_t allocate(storage_type& buf) const = 0;

  /**
   * Make sure we always have a virtual destructor to call.
   */
  virtual ~concept_base() {};
};
template <typename BaseModel>
using m_concept = typename BaseModel::concept_type;
template <typename BaseModel>
using m_storage = typename BaseModel::storage_type;

template <typename Tag, typename Base>
using link_concepts = typename Tag::template concept<Base>;

// user interface
template <typename Concept, typename AnyOptions>
using chain_concepts =
    compose_feature_groups_t<link_concepts,
                             compose_models_t,
                             typename AnyOptions::feature_groups,
                             concept_base<Concept, AnyOptions>>;
#if 0
template <typename Concept, typename AnyOptions>
using chain_concepts =
    multiply_inherit_all<meta::map_t<link_concepts,
                                     typename AnyOptions::tags,
                                     concept_base<Concept, AnyOptions>>>;
#endif

/* ****************************************************************
 * type function: concept
 * ****************************************************************/
template <typename AnyOptions>
struct concept_t : chain_concepts<concept_t<AnyOptions>, AnyOptions> { };

template <typename AnyOptions>
using any_concept = concept_t<AnyOptions>;

/* ******************************************************************
 * type function: chain_models
 * ******************************************************************/

/**
 * In the end, provide a few useful typedefs and self(), and inherit from
 * Concept to get all of the stuff from it.
 */
template <typename Value, typename Model, typename Concept>
struct model_base : Concept {
  using value_type = Value;
  using model_type = Model;

  model_type& self() { return static_cast<model_type&>(*this); }
  decltype(auto) value() { return self().value(); }
  decltype(auto) value() const { return self().value(); }

  model_type const& self() const {
    return static_cast<model_type const&>(*this);
  }
  model_type& self_cast(m_concept<Concept>& y) const {
    return dynamic_cast<model_type&>(y);
  }
  model_type const& self_cast(m_concept<Concept> const& y) const {
    return dynamic_cast<model_type const&>(y);
  }
  buffer_spec sizeof_alignof() const override final {
    return {sizeof(model_type), alignof(model_type)};
  }
  std::type_info const& target_type() const override final {
    return typeid(value_type);
  }
  buffer_t allocate(m_storage<Concept>& buf) const override final {
    return buf.template allocate<model_type>();
  }
};
template <typename BaseModel>
using m_value = typename BaseModel::value_type;
template <typename BaseModel>
using m_model = typename BaseModel::model_type;

template <typename Tag, typename Base>
using link_models = typename Tag::template model<Base>;
template <typename Tag, typename Base1, typename Base2>
using link_model_unhiders =
    typename Tag::template model_name_unhider<Base1, Base2>;

template <typename Value, typename Model, typename Concept>
using chain_models =
    compose_feature_groups_t<link_models,
                             compose_models_t,
                             typename Concept::options::feature_groups,
                             model_base<Value, Model, Concept>>;
#if 0
template <typename Value, typename Model, typename Concept>
using chain_models = chain_t<link_models,
                             typename Concept::options::tags,
                             model_base<Value, Model, Concept>>;
#endif

/** SUPPORT FOR std::reference_wrapper */
template <typename Value>
struct reference_type {
  using value_type = Value;
  using type = Value;
  using reference = type&;
  using const_reference = type const&;
};

template <typename Value>
struct reference_type<std::reference_wrapper<Value>> {
  using value_type = std::reference_wrapper<Value>;
  using type = Value;
  using reference = type&;
  using const_reference = type const&;
};

/* ******************************************************************
 * type function: model
 * ******************************************************************/
template <typename Value, typename Concept>
struct model_t
    : chain_models<Value, model_t<Value, Concept>, Concept> {
  // repeat base type...
  using base = chain_models<Value, model_t<Value, Concept>, Concept>;
  using typename base::value_type;
  using reference = typename reference_type<value_type>::reference;
  using const_reference = typename reference_type<value_type>::const_reference;
  using type = typename reference_type<value_type>::type;
  static_assert(std::is_same<value_type, Value>(),
                "chain_models bug: incorrect chaining of value_type.");

  model_t() {}
  model_t(value_type&& x) : value_(std::move(x)) {}
  model_t(value_type const& x) : value_(x) {}

  reference value() { return value_; }
  const_reference value() const { return value_; }

  value_type const* get() const { return &value_; }
  value_type* get() { return &value_; }

  value_type value_;
};

template <typename Value, typename Concept>
using any_model = model_t<Value, Concept>;


/* ******************************************************************
 * type function: chain_interfaces
 * ******************************************************************/

template <typename Tag, typename Base>
using link_interfaces = typename Tag::template interface<Base>;
template <typename Tag, typename Base1, typename Base2>
using link_interface_name_unhiders =
    typename Tag::template interface_name_unhider<Base1, Base2>;

// base of recursion
template <typename InterfaceSupport>
struct interface_base {
  /**
   * EVERYTHING IN THIS CLASS BECOMES PART OF THE ANY INTERFACE.
   * Therefore, keep it minimal to prevent clashes with other interface
   * implementations.
   */

  /// Private. Should only be referenced by erasure::features::ifs.
  using _any_ifc_type = InterfaceSupport;
};

// prevent clashes
template <typename T>
using ifc = typename std::decay_t<T>::_any_ifc_type;
template <typename T>
using ifc_interface = typename ifc<T>::interface_type;
template <typename T, typename V>
using ifc_model = typename ifc<T>::template model_type<V>;
template <typename T>
using ifc_concept = typename ifc<T>::concept_type;
template <typename T>
using ifc_tags = typename ifc<T>::tags;
template <typename T>
using ifc_any_type = typename ifc<T>::any_type;
template <typename T>
decltype(auto) ifc_self_cast(T& x) {
  return static_cast<ifc_any_type<T>&>(x);
}
template <typename T>
decltype(auto) ifc_self_cast(T const& x) {
  return static_cast<ifc_any_type<T> const&>(x);
}
template <typename T>
auto ifc_concept_ptr(T& x) {
  return concept_ptr(ifc_self_cast(x));
}
template <typename T>
auto ifc_concept_ptr(T const& x) {
  return concept_ptr(ifc_self_cast(x));
}

template <typename Interface, typename InterfaceSupport>
using chain_interfaces =
    compose_feature_groups_t<link_interfaces,
                             compose_interfaces_t,
                             typename InterfaceSupport::options::feature_groups,
                             interface_base<InterfaceSupport>>;
#if 0
template <typename Interface, typename InterfaceSupport>
using chain_interfaces = multiply_inherit_t<link_interfaces,
                                            typename InterfaceSupport::tags,
                                            interface_base<InterfaceSupport>>;
#endif

/* *****************************************************************
 * type function: interface_t
 * *****************************************************************/

struct copy_assignable;
struct copy_constructible;
struct move_assignable;
struct move_constructible;

// forward declare interface_t because its support class needs it.
template <bool, bool, bool, bool, typename>
struct interface_t;

template <typename Concept,
          template <typename> class Model,
          typename AnyType>
struct interface_t_support {
  using options = typename Concept::options;
  using tags = typename options::tags;
  static_assert(meta::is_typelist<tags>{},
                "Taglist needs to be a typelist!");
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
  using interface_incomplete = interface_t<is_move_constructible{},
                                           is_move_assignable{},
                                           is_copy_constructible{},
                                           is_copy_assignable{},
                                           interface_t_support>;
  using interface_type = interface_incomplete;

  // the complete type of the interface. A problem if used in certain contexts
  using interface_complete =
      chain_interfaces<interface_incomplete, interface_t_support>;
  // what any inherits from
  using base = interface_complete;
  using storage = m_storage<concept_type>;
};
template <typename Support, typename Any1>
typename Support::any_type& self_any_cast(Any1& x) {
  return static_cast<typename Support::any_type&>(x);
}
template <typename Support, typename Any1>
typename Support::any_type const& self_any_cast(Any1 const& x) {
  return static_cast<typename Support::any_type const&>(x);
}

/* forward-declare functions we need to implement interface_t. */
template <typename Interface>
auto concept_ptr(Interface& x);
template <typename Interface>
void reset(Interface& x);
template <typename Interface>
auto const& buffer_ref(Interface const& x);
template <typename Interface>
auto& buffer_ref(Interface& x);

template <typename AnyOptions>
struct any_t;
template <typename AnyOptions, typename T>
void create_any_from_value(any_t<AnyOptions>& x, T&& value);

template <typename Support, typename T>
using disable_if_same_any_type =
    std::enable_if_t<!Support::template is_self_type<T>::value>;
template <typename S>
struct creation_support : S::base {
  creation_support() = default;
  template <typename T, typename = disable_if_same_any_type<S, T>>
  creation_support(T&& value) {
    auto& any_this = static_cast<typename S::any_type&>(*this);
    create_any_from_value(any_this, std::forward<T>(value));
  }
  template <typename T, typename = disable_if_same_any_type<S, T>>
  typename S::any_type& operator=(T&& value) {
    auto& any_this = static_cast<typename S::any_type&>(*this);
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
any_t<AO>& move_construct_any(any_t<AO>& target, any_t<AO>&& source) {
  if (buffer_ref(source)) {
    concept_ptr(source)->allocate_and_move_construct_in(buffer_ref(target));
  }
  return target;
}
template <typename AO>
any_t<AO>& move_assign_any(any_t<AO>& target,
                           any_t<AO>&& source,
                           /* is_move_assignable */ std::false_type) {
  reset(target);
  move_construct_any(target, std::move(source));
  return target;
}
template <typename AO>
any_t<AO>& move_assign_any(any_t<AO>& target,
                           any_t<AO>&& source,
                           /* is_move_assignable */ std::true_type) {
  if (same_dynamic_type(target, source)) {
    concept_ptr(target)->move_assign(std::move(*concept_ptr(source)));
  } else {
    move_assign_any(target, std::move(source), std::false_type{});
  }
  return target;
}

// COPY IMPLEMENTATIONS
template <typename Any1, typename Any2>
void copy_construct_any(Any1& target, Any2 const& source) {
  if (buffer_ref(source)) {
    concept_ptr(source)->allocate_and_copy_construct_in(buffer_ref(target));
  }
}
/** For copy_assign_any, the last parameter is is_copyable */
template <typename Any1, typename Any2>
Any1& copy_assign_any(Any1& target, Any2 const& source, std::false_type) {
  reset(target);
  copy_construct_any(target, source);
  return target;
}
template <typename Any1, typename Any2>
Any1& copy_assign_any(Any1& target, Any2 const& source, std::true_type) {
  if (same_dynamic_type(target, source)) {
    concept_ptr(target)->copy_assign(*concept_ptr(source));
  } else {
    copy_assign_any(target, source, std::false_type{});
  }
  return target;
}

/*** INTERFACE_T INSTANTIATIONS ***/

#define INTERFACE_T_MOVE_CONSTRUCTOR                    \
  interface_t(interface_t&& x) {                        \
    move_construct_any(self_any_cast<S>(*this),         \
                       std::move(self_any_cast<S>(x))); \
  }                                                     \
  static_assert(true, "")

#define INTERFACE_T_COPY_CONSTRUCTOR                                  \
  interface_t(interface_t const& x) {                                 \
    copy_construct_any(self_any_cast<S>(*this), self_any_cast<S>(x)); \
  }                                                                   \
  static_assert(true, "")

#define INTERFACE_T_MOVE_ASSIGNMENT                           \
  interface_t& operator=(interface_t&& x) {                   \
    return move_assign_any(self_any_cast<S>(*this),           \
                           std::move(self_any_cast<S>(x)),    \
                           typename S::is_move_assignable{}); \
  }                                                           \
  static_assert(true, "")

#define INTERFACE_T_COPY_ASSIGNMENT                           \
  interface_t& operator=(interface_t const& x) {              \
    return copy_assign_any(self_any_cast<S>(*this),           \
                           self_any_cast<S>(x),               \
                           typename S::is_copy_assignable{}); \
  }                                                           \
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
  interface_t& operator=(interface_t const& x) = delete;
};
template <typename S>
struct interface_t<true, true, false, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  INTERFACE_T_MOVE_ASSIGNMENT;
  interface_t(interface_t const& x) = delete;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<true, true, false, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  INTERFACE_T_MOVE_ASSIGNMENT;
  interface_t(interface_t const& x) = delete;
  interface_t& operator=(interface_t const& x) = delete;
};
template <typename S>
struct interface_t<true, false, true, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  interface_t& operator=(interface_t&& x) = delete;
  INTERFACE_T_COPY_CONSTRUCTOR;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<true, false, true, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  interface_t& operator=(interface_t&& x) = delete;
  INTERFACE_T_COPY_CONSTRUCTOR;
  interface_t& operator=(interface_t const& x) = delete;
};
template <typename S>
struct interface_t<true, false, false, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  interface_t& operator=(interface_t&& x) = delete;
  interface_t(interface_t const& x) = delete;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<true, false, false, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  INTERFACE_T_MOVE_CONSTRUCTOR;
  interface_t& operator=(interface_t&& x) = delete;
  interface_t(interface_t const& x) = delete;
  interface_t& operator=(interface_t const& x) = delete;
};
template <typename S>
struct interface_t<false, true, true, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t&& x) = delete;
  INTERFACE_T_MOVE_ASSIGNMENT;
  INTERFACE_T_COPY_CONSTRUCTOR;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<false, true, true, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t&& x) = delete;
  INTERFACE_T_MOVE_ASSIGNMENT;
  INTERFACE_T_COPY_CONSTRUCTOR;
  interface_t& operator=(interface_t const& x) = delete;
};
template <typename S>
struct interface_t<false, true, false, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t&& x) = delete;
  INTERFACE_T_MOVE_ASSIGNMENT;
  interface_t(interface_t const& x) = delete;
  INTERFACE_T_COPY_ASSIGNMENT;
  static_assert(!std::is_same<S, S>{},
                "Types that are not either move or copy "
                "constructible are not supported.");
};
template <typename S>
struct interface_t<false, true, false, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t&& x) = delete;
  INTERFACE_T_MOVE_ASSIGNMENT;
  interface_t(interface_t const& x) = delete;
  interface_t& operator=(interface_t const& x) = delete;
  static_assert(!std::is_same<S, S>{},
                "Types that are not either move or copy "
                "constructible are not supported.");
};
template <typename S>
struct interface_t<false, false, true, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t&& x) = delete;
  interface_t& operator=(interface_t&& x) = delete;
  INTERFACE_T_COPY_CONSTRUCTOR;
  INTERFACE_T_COPY_ASSIGNMENT;
};
template <typename S>
struct interface_t<false, false, true, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t&& x) = delete;
  interface_t& operator=(interface_t&& x) = delete;
  INTERFACE_T_COPY_CONSTRUCTOR;
  interface_t& operator=(interface_t const& x) = delete;
};
template <typename S>
struct interface_t<false, false, false, true, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t&& x) = delete;
  interface_t& operator=(interface_t&& x) = delete;
  interface_t(interface_t const& x) = delete;
  INTERFACE_T_COPY_ASSIGNMENT;
  static_assert(!std::is_same<S, S>{},
                "Types that are not either move or copy "
                "constructible are not supported.");
};
template <typename S>
struct interface_t<false, false, false, false, S> : creation_support<S> {
  using creation_support<S>::creation_support;
  interface_t() = default;
  interface_t(interface_t&& x) = delete;
  interface_t& operator=(interface_t&& x) = delete;
  interface_t(interface_t const& x) = delete;
  interface_t& operator=(interface_t const& x) = delete;
  static_assert(!std::is_same<S, S>{},
                "Types that are not either move or copy "
                "constructible are not supported.");
};

#undef INTERFACE_T_COPY_ASSIGNMENT
#undef INTERFACE_T_MOVE_ASSIGNMENT
#undef INTERFACE_T_COPY_CONSTRUCTOR
#undef INTERFACE_T_MOVE_CONSTRUCTOR

struct interface_t_access {
  template <bool MC, bool MA, bool CC, bool CA, typename S>
  static auto& value_ptr(interface_t<MC, MA, CC, CA, S>& x) {
    return x._any_ifc_value;
  }
  template <bool MC, bool MA, bool CC, bool CA, typename S>
  static auto const& value_ptr(interface_t<MC, MA, CC, CA, S> const& x) {
    return x._any_ifc_value;
  }
};
template <typename Interface>
auto const& buffer_ref(Interface const& x) {
  return interface_t_access::value_ptr(x);
}
template <typename Interface>
auto& buffer_ref(Interface& x) {
  return interface_t_access::value_ptr(x);
}
template <typename Interface>
auto concept_ptr(Interface& x) {
  return reinterpret_cast<ifc_concept<decltype(x)>*>(buffer_ref(x).get());
}
template <typename Interface>
auto concept_ptr(Interface const& x) {
  return reinterpret_cast<ifc_concept<decltype(x)> const*>(buffer_ref(x).get());
}
template <typename T, typename Interface>
auto const* model_ptr(Interface const& x) {
  auto cptr = concept_ptr(x);
  auto const impl_p = dynamic_cast<ifc_model<decltype(x), T> const*>(cptr);
  return impl_p ? impl_p : nullptr;
}
template <typename T, typename Interface>
auto* model_ptr(Interface& x) {
  auto const& const_x = x;
  return const_cast<ifc_model<decltype(x), T>*>(model_ptr<T>(const_x));
}
template <typename Interface>
void reset(Interface& x) {
  using concept = ifc_concept<decltype(x)>;
  auto value = concept_ptr(x);
  if (value) {
    value->~concept();
    buffer_ref(x).reset();
  }
}
template <typename AO>
bool same_dynamic_type(any_t<AO> const& x, any_t<AO> const& y) {
  return same_dynamic_type(concept_ptr(x), concept_ptr(y));
}

/**
 * The any_interface constructor.
 * Calculates the support type for the interface, and the concept and model
 * types for it.
 */
template <typename AnyOptions>
struct any_interface_t {
  using tags = typename AnyOptions::tags;
  using concept = any_concept<AnyOptions>;
  template <typename T>
  using model = any_model<std::decay_t<T>, concept>;
  using support_type = interface_t_support<concept, model, any_t<AnyOptions>>;
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
using meta::typelist;
using meta::copy_if_t;

/** The option for the inner buffer size. */
template <std::size_t BufferSize>
struct buffer_size : std::integral_constant<std::size_t, BufferSize> {};
template <std::size_t Size>
struct feature_concept_check<buffer_size<Size>> {
  using type = std::true_type;
};
/** The predicate that tells us whether a tag is a buffer_size */
template <typename T> struct is_buffer_size : std::false_type {};
template <std::size_t BufferSize>
struct is_buffer_size<buffer_size<BufferSize>> : std::true_type {};

using meta::head_t;

using meta::find_first_t;
using meta::copy_if_not_t;
using meta::concatenate_t;
using meta::and_;
using meta::map_t;
using meta::foldl_t;

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
    std::conditional_t<std::is_same<T, U>{},
                       std::true_type,
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

template <typename AnyOptions>
struct any_t : any_interface<AnyOptions> {
  using _any_base = any_interface<AnyOptions>;
  // inherit constructors
  using _any_base::_any_base;
  using _any_base::operator=;
};
template <typename... Features>
using any = typename make_any_t<Features...>::type;

// needed for ADL-swap finding to work correctly.
template <typename AnyOptions>
void swap(any<AnyOptions>& x, any<AnyOptions>& y) {
  using std::swap;
  using base = ifc_interface<any<AnyOptions>>;
  swap((base&)x, (base&)y);
}

template <typename AnyOptions>
bool empty(any_t<AnyOptions> const& x) {
  return concept_ptr(x) == nullptr;
}

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
auto make_model(buffer_t storage, T&& x, std::true_type, U)
    -> m_concept<Model> * {
  using std::forward;
  using model = Model;

  assert(reinterpret_cast<std::intptr_t>(storage.data) % alignof(model) == 0);
  assert(sizeof(model) <= storage.size);

  return new (storage.data) model{forward<T>(x)};
}
template <typename Model, typename T>
auto make_model(buffer_t storage, T&& x, std::false_type, std::true_type)
    -> m_concept<Model> * {
  using model = Model;

  assert(reinterpret_cast<std::intptr_t>(storage.data) % alignof(model) == 0);
  assert(sizeof(model) <= storage.size);

  auto c_ptr = new (storage.data) model{};
  c_ptr->value() = std::forward<T>(x);
  return c_ptr;
}
template <typename Model, typename T>
auto make_model(buffer_t storage, T&& x, std::false_type, std::false_type)
    -> m_concept<Model> * {
  static_assert(!std::is_same<Model, Model>{},
                "To be move-assignable, a type needs to either "
                "be move constructible or "
                "default constructible.");
}
template <typename Model, typename T>
auto make_model(buffer_t storage, T&& x) -> m_concept<Model> * {
  return make_model<Model>(storage,
                           std::forward<T>(x),
                           std::is_move_constructible<T>{},
                           std::is_default_constructible<T>{});
}

template <typename AnyOptions, typename T>
void create_any_from_value(any_t<AnyOptions>& x, T&& value) {
  assert(empty(x));
  using model = ifc_model<decltype(x), T>;
  make_model<model>(buffer_ref(x).template allocate<model>(),
                    std::forward<T>(value));
}

template <typename... Tags, typename T>
auto make_any(T&& x) -> any<Tags...> {
  return{std::forward<T>(x)};
}

template <typename T, typename AnyOptions>
T const* target(any_t<AnyOptions> const& x) {
  using cast_to = ifc_model<decltype(x), T>;
  auto const impl_p = dynamic_cast<cast_to const*>(concept_ptr(x));
  return impl_p ? impl_p->get() : nullptr;
}
template <typename T, typename AnyOptions>
T* target(any_t<AnyOptions>& x) {
  auto const& const_x = x;
  return const_cast<T*>(target<T const>(const_x));
}
template <typename T, typename AnyOptions>
std::type_info const& target_type(any_t<AnyOptions> const& x) {
  auto const pc = concept_ptr(x);
  return pc ? pc->get_target_type() : typeid(void);
}

/* feature definition helper */
struct feature {
  struct empty_provides_tag {
    template <template <typename> class T, typename U>
    struct model_name_unhider : T<U> {};
    template <template <typename> class T, typename U>
    struct interface_name_unhider : T<U> {};
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
  struct concept : C {
    virtual void move_construct_in(buffer_t buf) = 0;
    virtual void allocate_and_move_construct_in(m_storage<C>& buf) = 0;
  };

  template <typename M>
  struct model : M {
    void move_construct_in(buffer_t buf) override final {
      make_model<m_model<M>>(buf, std::move(*M::self().get()));
    }
    void allocate_and_move_construct_in(m_storage<M>& buf) override final {
      move_construct_in(M::allocate(buf));
    }
  };

  template <typename I>
  struct interface : I {
    /** In interface_t, because the move constructor is a special method. */
  };
};

/* ***********************************************************
 * MOVE ASSIGNABLE
 * ***********************************************************/
struct move_assignable : feature {
  template <typename C>
  struct concept : C {
    virtual void move_assign(m_concept<C>&&) = 0;
  };

  template <typename M>
  struct model : M {
    void move_assign(m_concept<M>&& y) override final {
      M::value() = std::move(M::self_cast(y).value());
    }
  };

  template <typename I>
  struct interface : I {
    /** In interface_t, because move assignment is a special method. */
  };
};


/* ***********************************************************
 * COPYABLE
 * ***********************************************************/
struct copy_constructible : feature {
  template <typename C>
  struct concept : C {
    virtual void copy_construct_in(buffer_t) const = 0;
    virtual void allocate_and_copy_construct_in(m_storage<C>&) const = 0;
  };

  // model
  template <typename M>
  struct model : M {
    void copy_construct_in(buffer_t buffer) const override final {
      make_model<m_model<M>>(buffer, *M::self().get());
    }
    void allocate_and_copy_construct_in(m_storage<M>& buf) const override final {
      copy_construct_in(M::allocate(buf));
    }
  };

  template <typename I>
  struct interface : I {
    // implemented in interface_t because copy construction is special
  };
};

/* ***********************************************************
 * COPY_ASSIGNABLE
 * ***********************************************************/
struct copy_assignable : feature {
  template <typename C>
  struct concept : C {
    virtual void copy_assign(m_concept<C> const& x) = 0;
  };

  template <typename M>
  struct model : M {
    // precondition: y is of model_type.
    void copy_assign(m_concept<M> const& y) override final {
      *M::self().get() = *M::self_cast(y).get();
    }
  };

  template <typename I>
  struct interface : I {
    // copy-assignment is special, defined in interface_t
  };
};

/* ***************************************************************
 * SWAPPABLE
 * ***************************************************************/
struct swappable : feature {
  template <typename C>
  struct concept : C {
    virtual void swap(m_concept<C>& y) noexcept = 0;
  };

  template <typename M>
  struct model : M {
    // precondition: same type, ensured by swappable_interface
    void swap(m_concept<M>& y) noexcept override final {
      using std::swap;
      swap(M::value(), M::self_cast(y).value());
    }
  };

  template <typename I>
  struct interface : I {
    friend void swap(ifc_any_type<I>& x, ifc_any_type<I>& y) noexcept {
      if (same_dynamic_type(x, y)) {
        concept_ptr(x)->swap(*concept_ptr(y));
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
auto make_any_like(T&& x) {
  return make_any<typename get_options<AnyType>::all_tags>(std::forward<T>(x));
}

using movable = typelist<move_constructible, move_assignable>;
using copyable = typelist<copy_constructible, copy_assignable>;

namespace debug {
template <typename Options>
std::size_t model_size(any_t<Options> const& x) {
  auto const pc = concept_ptr(x);
  return pc ? pc->sizeof_alignof().size : 0;
}
} // debug

} // _1
// ################# END IMPLEMENTATION, BEGIN PUBLIC INTERFACE SYNOPSIS #######

namespace feature_support {
using _1::typelist;
using _1::ifc;
using _1::ifc_model;
using _1::ifc_concept;
using _1::ifc_interface;
using _1::ifc_any_type;
using _1::m_model;
using _1::m_value;
using _1::m_storage;
using _1::m_concept;
using _1::concept_ptr;
using _1::ifc_self_cast;
using _1::ifc_concept_ptr;
using _1::feature;
} // end erasure::feature_support

// import all the names that the library exports

// the any type constructor
using _1::any;
using _1::make_any;
using _1::make_any_like;

// the member access functions
using _1::target;
using _1::target_type;
using _1::empty;

namespace features {
// type tags implementation
using _1::move_assignable;
using _1::move_constructible;
using _1::copy_assignable;
using _1::copy_constructible;
using _1::swappable;
using _1::buffer_size;
// type tag sets implementation
using _1::copyable;
using _1::movable;
}


namespace debug = _1::debug;

} /* namespace erasure */
