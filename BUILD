cc_library(
    name = "erasure",
    hdrs = [
        "erasure/erasure.hpp",
        "erasure/feature/callable.hpp",
        "erasure/feature/dereferenceable.hpp",
        "erasure/feature/equality_comparable.hpp",
        "erasure/feature/less_than_comparable.hpp",
        "erasure/feature/ostreamable.hpp",
        "erasure/feature/regular.hpp",
        "erasure/feature/value_equality_comparable.hpp",
        "erasure/meta.hpp",
        "erasure/small_buffer.hpp",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "debug",
    testonly = True,
    hdrs = [
        "debug/atom.hpp",
        "debug/demangle.hpp",
        "debug/instrumented.hpp",
        "debug/unique_string.hpp",
    ],
    visibility = ["//visibility:public"],
)
