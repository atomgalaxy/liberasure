#pragma once

#include "equality_comparable.hpp"
#include "../type_erasure.hpp"

namespace type_erasure {
namespace features {
using regular =
    feature_support::typelist<movable, copyable, equality_comparable>;
}
}
