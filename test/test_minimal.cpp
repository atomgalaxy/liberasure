#include "erasure/erasure.hpp"
#include "erasure/feature/regular.hpp"

int main() {
  using erasure::any;
  namespace f = erasure::features;

  erasure::any<f::regular> x = 5;
}