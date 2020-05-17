load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_test(
    name = "open_file_test",
    srcs = ["open_file_test.cc"],
    deps = [
        "//proto:latis_msg_cc_proto",
        "//src/test_utils:test_utils_lib",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)
