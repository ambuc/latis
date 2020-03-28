load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//src:__subpackages__"])

cc_library(
    name = "cleanup",
    hdrs = ["cleanup.h"],
    deps = [],
)

cc_library(
    name = "status_macros",
    hdrs = ["status_macros.h"],
    deps = [
        "//proto:latis_msg_cc_proto",
    ],
)

cc_test(
    name = "status_macros_test",
    srcs = ["status_macros_test.cc"],
    deps = [
        ":status_macros",
        "//src/test_utils:test_utils_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
