cc_library(
    name = "erasure",
    hdrs = [
        "include/erasure/erasure.hpp",
        "include/erasure/feature/callable.hpp",
        "include/erasure/feature/dereferencable.hpp",
        "include/erasure/feature/equality_comparable.hpp",
        "include/erasure/feature/less_than_comparable.hpp",
        "include/erasure/feature/ostreamable.hpp",
        "include/erasure/feature/regular.hpp",
        "include/erasure/feature/value_equality_comparable.hpp",
        "include/erasure/meta.hpp",
        "include/erasure/small_buffer.hpp",
    ],
    strip_include_prefix = "include/",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "debug",
    testonly = True,
    hdrs = [
        "include/debug/atom.hpp",
        "include/debug/demangle.hpp",
        "include/debug/instrumented.hpp",
        "include/debug/unique_string.hpp",
    ],
    strip_include_prefix = "include/",
    visibility = ["//visibility:public"],
)
