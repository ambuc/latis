load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "display_utils_lib",
    srcs = ["display_utils.cc"],
    hdrs = ["display_utils.h"],
    deps = [
        ":xy_lib",
        "//proto:latis_msg_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/functional:bind_front",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
    ],
)

cc_test(
    name = "display_utils_test",
    srcs = ["display_utils_test.cc"],
    deps = [
        ":display_utils_lib",
        ":xy_lib",
        "//proto:latis_msg_cc_proto",
        "//src/test_utils:test_utils_lib",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "latis_interface",
    hdrs = ["latis_interface.h"],
    deps = [
        "//proto:latis_msg_cc_proto",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "latis_impl",
    srcs = ["latis_impl.cc"],
    hdrs = ["latis_impl.h"],
    deps = [
        ":display_utils_lib",
        ":latis_interface",
        ":xy_lib",
        "//proto:latis_msg_cc_proto",
        "//src/formula:common_lib",
        "//src/formula:evaluator_lib",
        "//src/formula:formula_lib",
        "//src/graph",
        "//src/utils:status_macros",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
    ],
)

cc_test(
    name = "latis_impl_test",
    srcs = ["latis_impl_test.cc"],
    deps = [
        ":display_utils_lib",
        ":latis_impl",
        ":latis_interface",
        "//proto:latis_msg_cc_proto",
        "//src/test_utils:test_utils_lib",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "latis",
    srcs = ["main.cc"],
    deps = [
        ":latis_impl",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
)

cc_library(
    name = "xy_lib",
    srcs = ["xy.cc"],
    hdrs = ["xy.h"],
    visibility = ["//src/formula:__subpackages__"],
    deps = [
        "//proto:latis_msg_cc_proto",
        "@com_google_absl//absl/hash",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "xy_test",
    srcs = ["xy_test.cc"],
    deps = [
        ":xy_lib",
        "//src/test_utils:test_utils_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
