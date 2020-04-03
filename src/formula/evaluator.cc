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

namespace {

Amount BoolToAmount(bool b) {
  Amount a;
  a.set_bool_amount(b);
  return a;
}
// Monadic shim.
StatusOr<Amount> BoolToAmount(StatusOr<bool> b) {
  if (b.ok()) {
    return BoolToAmount(b.ValueOrDie());
  }
  return b.status();
}

} // namespace

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
    return Status(INVALID_ARGUMENT,
                  absl::StrFormat("Evaluator: no value in cell %s", xy.ToA1()));
  }
  return maybe_value.value();
}

StatusOr<Amount> Evaluator::CrunchOperation(const Expression::Operation &op) {
  std::string_view fn_name = op.fn_name();

  switch (op.terms_size()) {
  case (1): {
    Amount arg;
    ASSIGN_OR_RETURN_(arg, CrunchExpression(op.terms(0)));
    if (fn_name == functions::kNOT) {
      return !arg;
    } else if (fn_name == functions::kNEG) {
      return -arg;
    }
  }
  case (2): {
    Amount lhs;
    ASSIGN_OR_RETURN_(lhs, CrunchExpression(op.terms(0)));
    Amount rhs;
    ASSIGN_OR_RETURN_(rhs, CrunchExpression(op.terms(1)));

    if (fn_name == functions::kPLUS || fn_name == functions::kSUM ||
        fn_name == functions::kADD) {
      return lhs + rhs;
    } else if (fn_name == functions::kMINUS || fn_name == functions::kSUB ||
               fn_name == functions::kSUBTRACT) {
      return lhs - rhs;
    } else if (fn_name == functions::kMULTIPLIED_BY ||
               fn_name == functions::kTIMES || fn_name == functions::kPRODUCT) {
      return lhs * rhs;
    } else if (fn_name == functions::kDIVIDED_BY ||
               fn_name == functions::kDIV) {
      return lhs / rhs;
    } else if (fn_name == functions::kAND) {
      return lhs && rhs;
    } else if (fn_name == functions::kOR) {
      return lhs || rhs;
    } else if (fn_name == functions::kLTHAN) {
      return BoolToAmount(lhs < rhs);
    } else if (fn_name == functions::kGTHAN) {
      return BoolToAmount(lhs > rhs);
    } else if (fn_name == functions::kLEQ) {
      return BoolToAmount(lhs <= rhs);
    } else if (fn_name == functions::kGEQ) {
      return BoolToAmount(lhs >= rhs);
    } else if (fn_name == functions::kEQ) {
      return BoolToAmount(lhs == rhs);
    } else if (fn_name == functions::kNEQ) {
      return BoolToAmount(lhs != rhs);
    } else if (fn_name == functions::kPOW) {
      return lhs ^ rhs;
    } else if (fn_name == functions::kMOD) {
      return lhs % rhs;
    }
  }
  default: {
    return Status(INVALID_ARGUMENT, " no operation match.");
  }
  }
}

} // namespace formula
} // namespace latis
