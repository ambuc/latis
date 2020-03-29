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

StatusOr<Amount> Evaluator::CrunchExpression(const Expression &expression) {
  // NB: no has_range.
  if (expression.has_value()) {
    return expression.value();
  } else if (expression.has_operation()) {
    return CrunchOperation(expression.operation());
  } else if (expression.has_lookup()) {
    return CrunchPointLocation(expression.lookup());
  } else {
    return Status(INVALID_ARGUMENT, "?");
  }
}

StatusOr<Amount>
Evaluator::CrunchPointLocation(const PointLocation &point_location) {
  const XY xy = XY::From(point_location);
  const absl::optional<Amount> maybe_value = lookup_fn_(xy);
  if (!maybe_value.has_value()) {
    return Status(INVALID_ARGUMENT, "no value.");
  }
  return maybe_value.value();
}

StatusOr<Amount>
Evaluator::CrunchOperation(const Expression::Operation &operation) {
  // TODO(ambuc): look up operation by name.
  return Status(INVALID_ARGUMENT, "");
}

} // namespace formula
} // namespace latis
