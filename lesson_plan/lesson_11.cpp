// lesson 11: chained type erasure

#if 0
Part 1: recap of type erasure: remember unsigned_property?

We had 3 parts to the type erasure:
 - interface (unsigned_property)
   This is the concrete handle on the stack.  It holds a handle to the
   unsigned_property_concept, and properly forwards operator[] to
   handle->operator_subscript(). The magic happens when we assign or construct
   the unsigned_property from an object that supports operator[]: an
   unsigned_property_model<T> will be created based on the T we are assigning
   or creating from, and the model will be put on the heap, along with a copy
   of that object.

 - unsigned_property_concept
   The abstract base class. Does not actually hold any value, on any state at
   all. All it does is define operator_subscript() as a virtual function.

 - unsigned_property_model<TypeWeAreHiding>
   The concrete class that implements the interface promised by
   unsigned_property_concept and forwards the function call to the 'value'
   member that it holds. 'value' is of type TypeWeAreHiding.

This allows us to achieve a type-erasure of anything that provides an
operator[].


Part 2: Recap of interface composition

We already said that inheritance is basically interface composition. So, let's
see how wse can compose abstract base classes this way.

#endif


// CRTP reminder

template <typename MyChild>
struct bar {
  void do_something() const {
    //static_cast<MyChild const&>(*this).baz;
  }
};

struct foo : bar<foo> {
  int baz;
};

void do_something_else() {
  foo x;
  x.do_something();
}


#include "../type_erasure/meta.hpp"

#include <cstddef>
#include <vector>
#include <utility>
#include <iostream>

struct sizable {
  template <typename Base>
  struct concept : Base {
    virtual size_t size() const = 0;
  };

  template <typename Base>
  struct model : Base {
    virtual size_t size() const override final { return Base::self().size(); }
  };

  template <typename Base>
  struct interface : Base {
    size_t size() const {
      return Base::handle()->size();
    }
  };
};

template <typename Key, typename Value>
struct mutably_subscriptable {

  template <typename Base>
  struct concept : Base {
    virtual Value& operator_subscript(Key const&) = 0;
    virtual Value& at(Key const&) = 0;
  };

  template <typename Base>
  struct model : Base {
    virtual Value& operator_subscript(Key const& key) override final {
      return Base::self()[key];
    }
    virtual Value& at(Key const& key) override final {
      return Base::self().at(key);
    }
  };

  template <typename Base>
  struct interface : Base {
    Value& operator[](Key const& key) {
      return Base::handle()->operator_subscript(key);
    }
    Value& at(Key const& key) {
      return Base::handle()->at(key);
    }
  };
};

template <typename Key, typename Value>
struct const_subscriptable {

  template <typename Base>
  struct concept : Base {
    virtual Value const& operator_subscript(Key const&) const = 0;
    virtual Value const& at(Key const&) const = 0;
  };

  template <typename Base>
  struct model : Base {
    virtual Value const& operator_subscript(
        Key const& key) const override final {
      return Base::self()[key];
    }
    virtual Value const& at(Key const& key) const override final {
      return Base::self().at(key);
    }
  };

  template <typename Base>
  struct interface : Base {
    Value const& operator[](Key const& key) const {
      return Base::handle()->operator_subscript(key);
    }
    Value const& at(Key const& key) const {
      return Base::handle()->at(key);
    }
  };
};

struct cloneable {
  template <typename Base>
  struct concept : Base {
    virtual std::unique_ptr<typename Base::concept_type> clone() const = 0;
  };

  template <typename Base>
  struct model : Base {
    virtual std::unique_ptr<typename Base::concept_type> clone()
        const override final {
      using model_type = typename Base::model_type;
      return std::make_unique<model_type>(
          static_cast<model_type const&>(*this));
    }
  };

  // this copy constructor and copy assignment need
  // to be implemented in the actual class
  template <typename Base>
  struct interface : Base {};
};

struct equality_comparable {
  template <typename Base>
  struct concept : Base {
    virtual bool compare_equal(typename Base::concept_type const&) const = 0;
  };

  template <typename Base>
  struct model : Base {
    // concept_type is *actually* our type. That's a precondition.
    virtual bool compare_equal(
        typename Base::concept_type const& x) const override final {
      auto const& other = static_cast<typename Base::model_type const&>(x);
      return other.self() == Base::self();
    }
  };

  // this copy constructor and copy assignment need
  // to be implemented in the actual class
  template <typename Base>
  struct interface : Base {
    friend bool operator==(typename Base::___interface_type const& x,
                           typename Base::___interface_type const& y) {
      if (typeid(*x.handle()) == typeid(*y.handle())) {
        return x.handle()->compare_equal(*y.handle());
      } else {
        return false;
      }
    }
    friend bool operator!=(typename Base::___interface_type const& x,
                           typename Base::___interface_type const& y) {
      return !(x == y);
    }
  };
};


#if 0
So, how do we compose these?
#endif

// our good friend...
template <typename ConceptType>
struct base_concept {
  using concept_type = ConceptType;
};

// this is a bit... cumbersome. Let's try this:
template <typename Feature, typename Base>
using apply_concept = typename Feature::template concept<Base>;

template <typename Features>
struct final_concept : meta::foldr_t<apply_concept,
                                     Features,
                                     base_concept<final_concept<Features>>> {};

// better, but we hardcoded the typelist - we can do it even better:
template <typename Features>
using concept = final_concept<Features>;

// the complex part comes when we want to write the feature implementations -
// namely, how do we get to value?

template <typename T, typename FinalModel, typename Concept>
struct base_model : Concept { // all models will inherit from this
  using model_type = FinalModel;
  // FinalModel, at this point, is an incomplete type.
  T& self() {
    return static_cast<FinalModel&>(*this).value;
  }
  T const& self() const {
    return static_cast<FinalModel const&>(*this).value;
  }
};

// ok, now we've got concepts covered. How do we do models?
// we're gonna need a final model that actually holds the value:
template <typename Feature, typename Base>
using apply_model = typename Feature::template model<Base>;

template <typename T, typename Features>
struct final_model final
    : meta::foldr_t<
          apply_model,
          Features,
          base_model<T, final_model<T, Features>, concept<Features>>> {

  final_model(T x) : value(std::move(x)) {}

  T value;
};

template <typename T, typename Features>
using model = final_model<T, Features>;

// we'll use this instead of the empty concept above.
//
// Now we can go back and redo the features

// lesson 12... roughly.
// Today, we do the interface.

template <typename FinalInterface>
struct base_interface {
  using ___interface_type = FinalInterface;

  auto handle() {
    return static_cast<FinalInterface&>(*this).handle_.get();
  }
  auto handle() const {
    return static_cast<FinalInterface const&>(*this).handle_.get();
  }
};

template <typename Feature, typename Base>
using apply_interface = typename Feature::template interface<Base>;

template <typename Features>
struct interface final : meta::foldr_t<apply_interface,
                                       Features,
                                       base_interface<interface<Features>>> {
  friend base_interface<interface<Features>>;
  interface() = default;
  interface(interface&&) = default;
  interface& operator=(interface&&) = default;
  interface(interface const& x) : handle_{x.handle_->clone()}
  {}

  interface& operator=(interface const& x) {
    handle_ = x.handle_->clone();
    return *this;
  }

  // construct from value
  template <typename T,
            typename =
                std::enable_if_t<(!std::is_same<interface, std::decay_t<T>>{})>>
  interface(T&& x)
      : handle_{std::make_unique<model<std::decay_t<T>, Features>>(
            std::forward<T>(x))} {}

  template <typename T,
            typename =
                std::enable_if_t<(!std::is_same<interface, std::decay_t<T>>{})>>
  interface& operator=(T&& x) {
    handle_ =
        std::make_unique<model<std::decay_t<T>, Features>>(std::forward<T>(x));
     return *this;
  }
private:
  std::unique_ptr<concept<Features>> handle_;
};

int main() {
  std::vector<unsigned> x{1,2,3,4,5};

  using features = meta::typelist<const_subscriptable<size_t, unsigned>,
                                  mutably_subscriptable<size_t, unsigned>,
                                  sizable, cloneable, equality_comparable>;
  // we can create a model
  interface<features> ifc{x};
  auto ifc2 = ifc;
  std::cout << ifc.size() << " " << ifc[2] << " " << (ifc2 == ifc) << "\n";

}

// today:
// - ref
// - move
// - copy
// - access

