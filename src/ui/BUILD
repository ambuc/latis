cc_library(
    name = "app_lib",
    srcs = ["app.cc"],
    hdrs = ["app.h"],
    linkopts = [
        "-lncursesw",
        "-lformw",
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
        "-lncursesw",
        "-lformw",
    ],
    deps = [
        ":common",
        ":window_lib",
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
    ],
)
