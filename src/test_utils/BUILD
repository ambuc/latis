load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//src:__subpackages__"])

filegroup(
    name = "textprotos",
    srcs = glob([
        "*.textproto",
    ]),
)

cc_library(
    name = "test_utils_lib",
    testonly = 1,
    hdrs = ["test_utils.h"],
    deps = [
        "//proto:latis_msg_cc_proto",
        "@com_google_googletest//:gtest_main",
    ],
)
