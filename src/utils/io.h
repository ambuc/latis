/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SRC_UTILS_IO_H_
#define SRC_UTILS_IO_H_

#include "proto/latis_msg.pb.h"
#include "src/utils/cleanup.h"

#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/statusor.h"
#include <fcntl.h>
#include <fstream>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <iostream>

namespace latis {

template <typename T>
::google::protobuf::util::StatusOr<T> FromTextproto(absl::string_view path) {
  if (path.empty()) {
    return ::google::protobuf::util::Status(
        ::google::protobuf::util::error::INVALID_ARGUMENT,
        "Can't parse a textproto from an empty path.");
  }
  int fd = open(std::string(path).c_str(), O_RDONLY);
  auto cleanup = MakeCleanup([&] { close(fd); });

  T parsed;

  google::protobuf::io::FileInputStream fstream(fd);
  if (!google::protobuf::TextFormat::Parse(&fstream, &parsed)) {
    return ::google::protobuf::util::Status(
        ::google::protobuf::util::error::INVALID_ARGUMENT,
        absl::StrFormat("Couldn't parse from %s", path));
  }

  return parsed;
}

} // namespace latis

#endif // SRC_UTILS_IO_H_
