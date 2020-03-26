load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "cell_interface",
    hdrs = ["cell_interface.h"],
    deps = [
        ":xy_lib",
        "//proto:latis_msg_cc_proto",
    ],
)

cc_library(
    name = "cell_impl",
    srcs = ["cell_impl.cc"],
    hdrs = ["cell_impl.h"],
    deps = [
        ":cell_interface",
        ":display_utils_lib",
        ":xy_lib",
        "//proto:latis_msg_cc_proto",
    ],
)

cc_test(
    name = "cell_impl_test",
    srcs = ["cell_impl_test.cc"],
    deps = [
        ":cell_impl",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "display_utils_lib",
    srcs = ["display_utils.cc"],
    hdrs = ["display_utils.h"],
    deps = [
        "//proto:latis_msg_cc_proto",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
    ],
)

cc_test(
    name = "display_utils_test",
    srcs = ["display_utils_test.cc"],
    deps = [
        ":display_utils_lib",
        "//proto:latis_msg_cc_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "metadata_interface",
    hdrs = ["metadata_interface.h"],
    deps = [
        "//proto:latis_msg_cc_proto",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "metadata_impl",
    srcs = ["metadata_impl.cc"],
    hdrs = ["metadata_impl.h"],
    deps = [":metadata_interface"],
)

cc_test(
    name = "metadata_impl_test",
    srcs = ["metadata_impl_test.cc"],
    deps = [
        ":metadata_impl",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "latis_interface",
    hdrs = ["latis_interface.h"],
    deps = [
        ":cell_impl",
        ":metadata_interface",
        "//proto:latis_msg_cc_proto",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "latis_impl",
    srcs = ["latis_impl.cc"],
    hdrs = ["latis_impl.h"],
    deps = [
        ":latis_interface",
        ":metadata_impl",
        ":xy_lib",
        "//proto:latis_msg_cc_proto",
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
        ":latis_impl",
        ":latis_interface",
        "//proto:latis_msg_cc_proto",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
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
