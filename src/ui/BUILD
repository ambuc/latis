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
