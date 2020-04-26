cc_library(
    name = "app_lib",
    srcs = ["app.cc"],
    hdrs = ["app.h"],
    linkopts = [
        "-lncurses",
    ],
    visibility = ["//src:__subpackages__"],
    deps = [
        ":common",
        ":widget_lib",
        ":window_lib",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_library(
    name = "common",
    hdrs = ["common.h"],
    visibility = ["//src:__subpackages__"],
    deps = [
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_library(
    name = "layout_engine",
    srcs = ["layout_engine.cc"],
    hdrs = ["layout_engine.h"],
    deps = [
        ":common",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_test(
    name = "layout_engine_test",
    srcs = ["layout_engine_test.cc"],
    deps = [
        ":layout_engine",
        "//src/test_utils:test_utils_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "widget_lib",
    srcs = ["widget.cc"],
    hdrs = ["widget.h"],
    linkopts = [
        "-lncurses",
        "-lform",
    ],
    deps = [
        ":common",
        ":window_lib",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "window_lib",
    srcs = ["window.cc"],
    hdrs = ["window.h"],
    linkopts = [
        "-lncurses",
    ],
    deps = [
        ":common",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)
