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
#include "src/formula/common.h"
#include "src/formula/functions.h"
#include "src/utils/status_macros.h"

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

  switch (operation.terms_size()) {
  case (1): {
    Amount arg;
    ASSIGN_OR_RETURN_(arg, CrunchExpression(operation.terms(0)));
    if (operation.fn_name() == functions::kNOT) {
      return !arg;
    } else if (operation.fn_name() == functions::kNEG) {
      return -arg;
    }
  }
  case (2): {
    Amount lhs;
    ASSIGN_OR_RETURN_(lhs, CrunchExpression(operation.terms(0)));
    Amount rhs;
    ASSIGN_OR_RETURN_(rhs, CrunchExpression(operation.terms(1)));

    if (operation.fn_name() == functions::kPLUS ||
        operation.fn_name() == functions::kSUM ||
        operation.fn_name() == functions::kADD) {
      return lhs + rhs;
    } else if (operation.fn_name() == functions::kMINUS ||
               operation.fn_name() == functions::kSUB ||
               operation.fn_name() == functions::kSUBTRACT) {
      return lhs - rhs;
    } else if (operation.fn_name() == functions::kMULTIPLIED_BY ||
               operation.fn_name() == functions::kTIMES ||
               operation.fn_name() == functions::kPRODUCT) {
      return lhs * rhs;
    } else if (operation.fn_name() == functions::kDIVIDED_BY ||
               operation.fn_name() == functions::kDIV) {
      return lhs / rhs;
    } else if (operation.fn_name() == functions::kAND) {
      return lhs && rhs;
    } else if (operation.fn_name() == functions::kOR) {
      return lhs || rhs;
    } else if (operation.fn_name() == functions::kLTHAN) {
      bool lthan;
      ASSIGN_OR_RETURN_(lthan, lhs < rhs);
      Amount resultant;
      resultant.set_bool_amount(lthan);
      return resultant;
    } else if (operation.fn_name() == functions::kGTHAN) {
      bool gthan;
      ASSIGN_OR_RETURN_(gthan, lhs > rhs);
      Amount resultant;
      resultant.set_bool_amount(gthan);
      return resultant;
    } else if (operation.fn_name() == functions::kLEQ) {
      bool leq;
      ASSIGN_OR_RETURN_(leq, lhs <= rhs);
      Amount resultant;
      resultant.set_bool_amount(leq);
      return resultant;
    } else if (operation.fn_name() == functions::kGEQ) {
      bool geq;
      ASSIGN_OR_RETURN_(geq, lhs >= rhs);
      Amount resultant;
      resultant.set_bool_amount(geq);
      return resultant;
    } else if (operation.fn_name() == functions::kEQ) {
      bool eq;
      ASSIGN_OR_RETURN_(eq, lhs == rhs);
      Amount resultant;
      resultant.set_bool_amount(eq);
      return resultant;
    } else if (operation.fn_name() == functions::kNEQ) {
      bool neq;
      ASSIGN_OR_RETURN_(neq, lhs != rhs);
      Amount resultant;
      resultant.set_bool_amount(neq);
      return resultant;
    }
  }
  default: {
    return Status(INVALID_ARGUMENT, " no operation match.");
  }
  }
}

} // namespace formula
} // namespace latis
