load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//src:__subpackages__"])

cc_library(
    name = "common_lib",
    hdrs = ["common.h"],
    deps = [
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "evaluator_lib",
    srcs = ["evaluator.cc"],
    hdrs = ["evaluator.h"],
    deps = [
        "//proto:latis_msg_cc_proto",
        "//src:xy_lib",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_test(
    name = "evaluator_test",
    srcs = ["evaluator_test.cc"],
    deps = [
        ":evaluator_lib",
        ":lexer_lib",
        ":parser_lib",
        "//src/test_utils:test_utils_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "lexer_lib",
    srcs = ["lexer.cc"],
    hdrs = ["lexer.h"],
    deps = [
        ":common_lib",
        "//proto:latis_msg_cc_proto",
        "//src/utils:status_macros",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_test(
    name = "lexer_test",
    srcs = ["lexer_test.cc"],
    deps = [
        ":lexer_lib",
        "//src/test_utils:test_utils_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "parser_combinators_lib",
    hdrs = ["parser_combinators.h"],
    deps = [
        ":common_lib",
        "//proto:latis_msg_cc_proto",
        "//src:xy_lib",
        "//src/utils:status_macros",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "parser_combinators_test",
    srcs = ["parser_combinators_test.cc"],
    deps = [
        ":parser_combinators_lib",
        "//src/test_utils:test_utils_lib",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "parser_lib",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        ":common_lib",
        ":parser_combinators_lib",
        "//proto:latis_msg_cc_proto",
        "//src:xy_lib",
        "//src/utils:cleanup",
        "//src/utils:status_macros",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/functional:bind_front",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "parser_test",
    srcs = ["parser_test.cc"],
    deps = [
        ":lexer_lib",
        ":parser_lib",
        "//src/test_utils:test_utils_lib",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)
