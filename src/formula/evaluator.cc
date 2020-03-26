// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/formula/evaluator.h"

#include "proto/latis_msg.pb.h"

#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"

namespace latis {
namespace formula {

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;
using ::google::protobuf::util::error::OK;

Status Evaluate(const Expression &expression, LookupFn lookup_fn,
                Amount *amount) {

  if (expression.has_value()) {
    *amount = expression.value();
    return Status(OK, "");
  }

  return Status(INVALID_ARGUMENT, "Evaluate() got no match.");
}

} // namespace formula
} // namespace latis
