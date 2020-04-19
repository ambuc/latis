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
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)
