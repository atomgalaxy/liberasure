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

/**
 *
 *                       AN ACTUALLY FUNCTIONAL REFERENCE
 *                          (or: practical interfaces)
 *
 *                           by: Gašper Ažman
 *
 */

#include "type_erasure/meta.hpp"

#include <functional>
#include <iostream>
#include <array>
#include <cassert>
#include <utility>
#include <type_traits>
#include <memory>


template <typename T>
using concept_type = typename T::concept_type;

template <typename... Bases>
struct concept : Bases::template concept<concept<Bases...>>... {
  using concept_type = concept;
};

template <typename CRTP, typename Base>
struct model_base : Base {
  using model_type = CRTP;
  using concept_type = Base;

  auto const& self() const { return static_cast<model_type const&>(*this); }
  auto& self() { return static_cast<model_type&>(*this); }
  auto const& v() const { return self().value; }
  auto& v() { return self().value; }
};

template <typename T, typename B>
using model_inherit = typename T::template model<B>;

template <typename T, typename... Bases>
struct model
    : meta::foldr_t<model_inherit,
                    meta::typelist<Bases...>,
                    model_base<model<T, Bases...>, concept<Bases...>>> {
  using typename concept<Bases...>::concept_type;

  model(T&& x) : value{std::move(x)} {}
  model(T const& x) : value{x} {}

  T value;
};

template <template <typename T> class Model, typename T>
auto create_model(T&& x) {
  using type = std::decay_t<T>;
  return std::make_unique<Model<type>>(std::forward<T>(x));
}

template <typename... Bases>
struct any : Bases::template interface<any<Bases...>>... {

  using concept_t = concept<Bases...>;

  template <typename T>
  using model_t = model<T, Bases...>;

  template <
      typename T,
      typename = std::enable_if_t<!std::is_same<std::decay_t<T>, any>{}>>
  any(T&& x)
      : h{create_model<model_t>(std::forward<T>(x))} {}

  auto& handle() { return *h; }
  auto const& handle() const { return *h; }

  std::unique_ptr<concept_t> h;
};


// --------------

template <typename Signature>
struct callable;

template <typename Return, typename... Args>
struct callable<Return(Args...)> {
  template <typename CRTP>
  struct concept {
    virtual Return operator()(Args... xs) const = 0;
  };

  template <typename B>
  struct model : B {
    virtual Return operator()(Args... xs) const override {
      return B::v()(xs...);
    }
  };

  template <typename CRTP>
  struct interface {
    Return operator()(Args... xs) const {
      auto const& x = static_cast<CRTP const&>(*this).handle();
      return x(xs...);
    }
  };

};


struct equality_comparable {
  template <typename CRTP>
  struct concept {
    virtual bool operator==(CRTP const& y) const = 0;
  };

  template <typename B>
  struct model : B {
    virtual bool operator==(
        typename B::concept_type const& y) const override {
      auto const& other = static_cast<typename B::model_type const&>(y);
      auto const& self = static_cast<typename B::model_type const&>(*this);
      return self.v() == other.v();
    }
  };

  template <typename CRTP>
  struct interface {
    friend bool operator==(CRTP const& x, CRTP const& y) {
      if (typeid(x) == typeid(y)) {
        return x.handle().operator==(y.handle());
      } else {
        return false;
      }
    }
  };
};

// ---------------------------------
struct foo {
  int eggs;

  template <typename... Ts>
  auto operator()(Ts... xs) const {
    // evaluate printing
    std::cout << "Granny has " << eggs << " eggs";
    auto _ = std::array<int, sizeof...(Ts)>{
        {((std::cout << " and also granny has " << xs << " eggs"), 0)...}};
    if (sizeof...(Ts) == 0) {
      std::cout << " because she is forgetful.\n";
    } else {
      std::cout << ".\n";
    }
    (void)_;
  }

  friend bool operator==(foo const& x, foo const& y) {
    return x.eggs == y.eggs;
  }
};


int main() {
  foo x{5};
  foo y{6};

  (void) (x == y); // this works

  using myref = any<callable<void()>,
                    callable<void(int)>,
                    callable<void(int, long)>,
                    equality_comparable>;
  myref xx{x};
  myref yy{y};

  xx();
  xx(1);
  xx(1, 2);

  assert(!(xx == yy));
  assert(xx == xx);
}
