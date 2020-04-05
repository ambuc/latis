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

#ifndef SRC_UTILS_STATUS_MACROS_H_
#define SRC_UTILS_STATUS_MACROS_H_

#include "proto/latis_msg.pb.h"

#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/status_macros.h"
#include "google/protobuf/stubs/statusor.h"

namespace latis {

// A lot of the stuff in
// https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/stubs/status_macros.h
// is permanently broken for me. Haven't figured out why yet. I think I don't
// have PROTOBUF_PREDICT_FALSE working yet. Here are some reimpls.

#define RETURN_IF_ERROR_(expr, ...)                                            \
  ({                                                                           \
    auto _expr_result = (expr);                                                \
    if (!_expr_result.ok()) {                                                  \
      return _expr_result;                                                     \
    }                                                                          \
  })

template <typename T>
::google::protobuf::util::Status
DoAssignOrReturn_(T &lhs, ::google::protobuf::util::StatusOr<T> result) {
  if (result.ok()) {
    lhs = result.ValueOrDie();
  }
  return result.status();
}

#define ASSIGN_OR_RETURN_(lhs, expr)                                           \
  ({                                                                           \
    ::google::protobuf::util::Status status = DoAssignOrReturn_(lhs, (expr));  \
    if (!status.ok()) {                                                        \
      return status;                                                           \
    }                                                                          \
  })

} // namespace latis

#endif // SRC_UTILS_STATUS_MACROS_H_
