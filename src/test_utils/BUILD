load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//src:__subpackages__"])

cc_library(
    name = "test_utils_lib",
    testonly = 1,
    hdrs = ["test_utils.h"],
    deps = [
        "@com_google_googletest//:gtest_main",
    ],
)
