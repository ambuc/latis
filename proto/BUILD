load("@rules_proto//proto:defs.bzl", "proto_library")

package(default_visibility = ["//src:__subpackages__"])

proto_library(
    name = "latis_msg_proto",
    srcs = ["latis_msg.proto"],
    deps = [
        # View all with bazel query:
        # 'kind(proto_library,  @com_google_protobuf//:all)'
        "@com_google_protobuf//:timestamp_proto",
    ],
)

cc_proto_library(
    name = "latis_msg_cc_proto",
    deps = [":latis_msg_proto"],
)
