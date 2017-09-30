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
 * TALK: mixins in C++.
 *
 * Or how I learned to stop worrying and love currying.
 *
 * What are mixins?
 * Mixins are classes that add functionality to the public interface of a
 * class.
 *
 * In our case, we want to define a grandma who has specific superpositions of
 * forgetfullnes of the number of eggs she has in her chicken coop.
 *
 * First, we'll define granny. She knows the real number of eggs. However, on
 * bad days, the environment bugs her brain and she also remembers a different
 * number.
 *
 * Now, to define granny's inheritance. We need to inherit from all the
 * single-parameter classes we got, and we need to pass them our own incomplete
 * type.
 *
 * So, this works. (compile, run)
 *
 * -----------------------------
 * Any questions?
 * -----------------------------
 *
 * Let's try to generalize these two days to be able to do what's in main(). We
 * want to define a really potentially forgetful grandma :).
 *
 * Take a look at the operator() first, so that it stops being misterious.
 */

#include <cassert>
#include <iostream>
#include <array>

#define TRY 1

#if TRY == 0
/**
 * Just to get something working.
 */
template <typename CRTP>
struct good_days {
  auto operator()() const {
    auto eggs = static_cast<CRTP const*>(this)->eggs;
    std::cout << "Granny has " << eggs << " eggs.\n";
  };
};

template <typename CRTP>
struct bad_days {
  template <typename T>
  auto operator()(T x) const {
    auto eggs = static_cast<CRTP const*>(this)->eggs;
    std::cout << "Granny has " << eggs << " eggs and also " << x
              << " eggs because she is forgetful.\n";
  };
};

template <template <typename> class... Bases>
struct granny : Bases<granny<Bases...>>... {
  granny(int e) : eggs{e} {}
  int eggs;
};

int main() {
  using big_g = granny<good_days, bad_days>;
  big_g grangran{2};
  grangran();
  grangran(3);
}

#elif TRY == 1
/**
 * But granny expects a very, very specific kind of thing to inherit from.
 * We need to be able to curry the type parameters.
 */
template <typename... Days>
struct all_days {
  template <typename CRTP>
  struct mixin {

    auto operator()(Days... xs) const {
      auto eggs = static_cast<CRTP const*>(this)->eggs;

      // evaluate printing
      std::cout << "Granny has " << eggs << " eggs ";
      (void)std::array<int, sizeof...(Days)>{
          {((std::cout << "and also granny has " << xs << " eggs"), 0)...}};
      if (sizeof...(Days) == 0) {
        std::cout << " because she is forgetful.\n";
      } else {
        std::cout << ".\n";
      }
    }

  };
};

struct grandma_equality {
  template <typename CRTP>
  struct mixin {
    friend bool operator==(CRTP const& x, CRTP const& y) {
      std::cout << "Granny x: " << x.eggs << " Granny y: " << y.eggs << "\n";
      return x.eggs == y.eggs;
    }
  };
};

template <typename... Bases>
struct granny : Bases::template mixin<granny<Bases...>>... {
  granny(int e) : eggs{e} {}
  int eggs;
};

int main() {
  using big_g = granny<all_days<>,
                       all_days<int>,
                       all_days<int, int>,
                       all_days<int, int, int>,
                       all_days<int, int, int, int>,
                       grandma_equality>;
  big_g grangran{2};
  grangran();
  grangran(3);
  grangran(3, 4);
  grangran(3, 4, 5);
  grangran(3, 4, 5, 6);
  (void) (grangran == grangran);
  static_assert(sizeof(grangran) == sizeof(int), "Look ma, no overhead");
}

#endif
