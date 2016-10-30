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

// lecture 09
#include <string>
#include <vector>
#include <unique_ptr>

#include "../type_erasure/meta.hpp"

struct empty {};

template <typename Base>
struct boat_concept : Base {
  virtual void sink() = 0;
};

// CRTP
template <typename ModelImpl, typename Base>
struct boat_model : Base {
  // assumes that ModelImpl inherits from boat_model
  void sink() override final {
    static_cast<ModelImpl*>(this)->value.sink();
  }
};

template <typename AnyImpl>
struct boat_interface {
  void sink() {
    static_cast<AnyImpl*>(this)->model_ptr->sink();
  }
};

template <typename Base>
struct any_concept : Base {};

template <typename T,
          template <typename, typename> class Base,
          typename Concept>
struct any_model
    : Base<any_model<T, Base, Concept>, Concept> {
  T value;
};

template <typename... Concepts>
struct any : boat_interface<any<Concepts...>> {
  using CONCEPT = any_concept<boat_concept<empty>>;
  template <typename T>
  using MODEL = any_model<T, boat_model, CONCEPT>;
  std::unique_ptr<CONCEPT> model_ptr;
};

struct dinghy {
  void sink() {}
};
struct tanker {
  void sink() {}
};

// the boat CONCEPT means you have
// void sink()

int main() {
  any<copyable, boat> x = dinghy{};
  any<copyable, boat> y = tanker{};
  y = x;
  y.sink();
  x.sink();
}
