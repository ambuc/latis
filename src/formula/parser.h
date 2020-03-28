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

#ifndef SRC_PARSER_PARSER_H_
#define SRC_PARSER_PARSER_H_

#include "proto/latis_msg.pb.h"

#include "src/formula/common.h"
#include "src/formula/parser_combinators.h"
#include "src/status_utils/status_macros.h"

#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/time/civil_time.h"
#include "absl/time/time.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "absl/types/variant.h"
#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/status_macros.h"
#include "google/protobuf/stubs/statusor.h"

namespace latis {
namespace formula {

enum class Step {
  kExpression,
  kOpTernary,
  kOpBinary,
  kOpBinaryText,
  kOpBinaryInfix,
  kOpUnary,
  kAmount,
};
std::string Print(Step s) {
  switch (s) {
  case (Step::kExpression):
    return "expression  ";
  case (Step::kOpTernary):
    return "ternary     ";
  case (Step::kOpBinary):
    return "binary      ";
  case (Step::kOpBinaryText):
    return "binary_text ";
  case (Step::kOpBinaryInfix):
    return "binary_infix";
  case (Step::kOpUnary):
    return "unary       ";
  case (Step::kAmount):
    return "amount      ";
  }
  return "";
}

using CacheItem = std::tuple<Step, TSpan::pointer, TSpan::size_type>;
using Cache = absl::flat_hash_set<CacheItem>;

Cache *GetCache() {
  static Cache cache{};
  return &cache;
}

Status RepeatGuard(Step step, TSpan *tspan) {
  // std::cout << Print(step) << "\t<-" << tspan->data() << ":\t";
  // PrintTSpan(tspan);

  CacheItem item = {step, tspan->data(), tspan->size()};

  if (GetCache()->contains(item)) {
    return Status(::google::protobuf::util::error::INVALID_ARGUMENT,
                  "RepeatGuard denied! Already been here");
  }
  GetCache()->insert(item);

  return OkStatus();
}

// Consumes the token |type| off |tspan| and returns the token's held
// |.value|.
StatusOr<std::string_view> ConsumeExact(Token::T type, TSpan *tspan);

// Consumes a |Token::T::numeric| token off |tspan| and parses it as an
// integer.
StatusOr<int> ConsumeInt(TSpan *tspan);

StatusOr<double> ConsumeDouble(TSpan *tspan);

StatusOr<absl::variant<double, int>> ConsumeNumeric(TSpan *tspan) {
  return AnyVariant<double, int>(ConsumeDouble, ConsumeInt)(tspan);
}

// TODO(ambuc): This could return a string view into the underlying
// string.
StatusOr<std::string> ConsumeString(TSpan *tspan);

StatusOr<int> Consume2Digit(TSpan *tspan);

StatusOr<int> Consume4Digit(TSpan *tspan);

// Expects "USD" or "CAD"
StatusOr<Money::Currency> ConsumeCurrencyWord(TSpan *tspan);
// Expects "$"
StatusOr<Money::Currency> ConsumeCurrencySymbol(TSpan *tspan);
StatusOr<Money::Currency> ConsumeCurrency(TSpan *tspan) {
  return Any<Money::Currency>({ConsumeCurrencySymbol, ConsumeCurrencyWord})(
      tspan);
}

StatusOr<Money> ConsumeMoney(TSpan *tspan);

StatusOr<int> ConsumeDateFullYear(TSpan *tspan) { return Consume4Digit(tspan); }

StatusOr<int> ConsumeDateMonth(TSpan *tspan) {
  static auto r = [](int i) { return 1 <= i && i <= 12; };
  return WithRestriction<int>(r)(Consume2Digit)(tspan);
}

StatusOr<int> ConsumeDateMDay(TSpan *tspan) {
  static auto r = [](int i) { return 1 <= i && i <= 31; };
  return WithRestriction<int>(r)(Consume2Digit)(tspan);
}

StatusOr<int> ConsumeTimeHour(TSpan *tspan) {
  static auto r = [](int i) { return 0 <= i && i <= 23; };
  return WithRestriction<int>(r)(Consume2Digit)(tspan);
}

StatusOr<int> ConsumeTimeMinute(TSpan *tspan) {
  static auto r = [](int i) { return 0 <= i && i <= 59; };
  return WithRestriction<int>(r)(Consume2Digit)(tspan);
}

StatusOr<int> ConsumeTimeSecond(TSpan *tspan) {
  // Up to 60, counting leap seconds.
  static auto r = [](int i) { return 0 <= i && i <= 60; };
  return WithRestriction<int>(r)(Consume2Digit)(tspan);
}

StatusOr<double> ConsumeTimeSecFrac(TSpan *tspan) {
  return ConsumeDouble(tspan);
}
StatusOr<absl::Time> ConsumeDateTime(TSpan *tspan);
StatusOr<absl::TimeZone> ConsumeTimeOffset(TSpan *tspan);

StatusOr<Amount> ConsumeAmount(TSpan *tspan);
StatusOr<int> ConsumeRowIndicator(TSpan *tspan);
StatusOr<int> ConsumeColIndicator(TSpan *tspan);
StatusOr<RangeLocation> ConsumeRangeLocationPointThenAny(TSpan *tspan);
StatusOr<RangeLocation> ConsumeRangeLocationRowThenRow(TSpan *tspan);
StatusOr<RangeLocation> ConsumeRangeLocationColThenCol(TSpan *tspan);
StatusOr<PointLocation> ConsumePointLocation(TSpan *tspan);
StatusOr<RangeLocation> ConsumeRangeLocation(TSpan *tspan) {
  return Any<RangeLocation>({ConsumeRangeLocationPointThenAny,
                             ConsumeRangeLocationRowThenRow,
                             ConsumeRangeLocationColThenCol})(tspan);
}
StatusOr<Expression> ConsumeParentheses(TSpan *tspan);
// [A-Z0-9_]
StatusOr<std::string> ConsumeFnName(TSpan *tspan);
StatusOr<Expression::OpUnary> ConsumeOpUnaryText(TSpan *tspan);
StatusOr<Expression::OpUnary> ConsumeOpUnary(TSpan *tspan) {
  return Any<Expression::OpUnary>({ConsumeOpUnaryText})(tspan);
}
StatusOr<Expression::OpBinary> ConsumeOpBinaryText(TSpan *tspan);
// Consumes "+", "-", "/", "*", "%", etc. and returns the string version for
// prefix notation.
StatusOr<std::string> ConsumeOpBinaryInfixFn(TSpan *tspan);
StatusOr<Expression::OpBinary> ConsumeOpBinaryInfix(TSpan *tspan);
StatusOr<Expression::OpBinary> ConsumeOpBinary(TSpan *tspan) {
  return Any<Expression::OpBinary>({ConsumeOpBinaryText, ConsumeOpBinaryInfix})(
      tspan);
}
StatusOr<Expression::OpTernary> ConsumeOpTernary(TSpan *tspan);

StatusOr<Expression> ConsumeExpression(TSpan *tspan);

} // namespace formula
} // namespace latis

#endif // SRC_PARSER_PARSER_H_
