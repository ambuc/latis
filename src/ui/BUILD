cc_library(
    name = "app_lib",
    srcs = ["app.cc"],
    hdrs = ["app.h"],
    linkopts = [
        "-lncursesw",
    ],
    visibility = ["//src:__subpackages__"],
    deps = [
        ":common",
        ":widget_lib",
        ":window_lib",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_library(
    name = "color",
    srcs = ["color.cc"],
    hdrs = ["color.h"],
    deps = [],
)

cc_library(
    name = "common",
    srcs = ["common.cc"],
    hdrs = ["common.h"],
    linkopts = [
        "-lncursesw",
    ],
    visibility = ["//src:__subpackages__"],
    deps = [
        ":color",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_library(
    name = "layout_engine",
    srcs = ["layout_engine.cc"],
    hdrs = ["layout_engine.h"],
    visibility = ["//src:__subpackages__"],
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
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "formwidget_lib",
    srcs = ["formwidget.cc"],
    hdrs = ["formwidget.h"],
    visibility = ["//src:__subpackages__"],
    deps = [":widget_lib"],
)

cc_library(
    name = "textwidget_lib",
    srcs = ["textwidget.cc"],
    hdrs = ["textwidget.h"],
    visibility = ["//src:__subpackages__"],
    deps = [
        ":formwidget_lib",
        ":widget_lib",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "gridwidget_lib",
    srcs = ["gridwidget.cc"],
    hdrs = ["gridwidget.h"],
    visibility = ["//src:__subpackages__"],
    deps = [
        ":textwidget_lib",
        ":widget_lib",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_library(
    name = "widget_lib",
    srcs = ["widget.cc"],
    hdrs = ["widget.h"],
    linkopts = [
        "-lncursesw",
        "-lformw",
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
        "-lncursesw",
    ],
    deps = [
        ":common",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
    ],
)
