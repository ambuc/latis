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
#include "src/utils/cleanup.h"
#include "src/utils/status_macros.h"

#include "absl/container/flat_hash_set.h"
#include "absl/functional/bind_front.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"
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

class Parser {
public:
  struct Options {
    bool should_log_verbosely{false};
  };

  // ctors
  Parser() : Parser(/*options=*/Options{}) {}
  explicit Parser(Options options) : options_(options) {}

  void EnableVerboseLogging() { options_.should_log_verbosely = true; }
  void DisableVerboseLogging() { options_.should_log_verbosely = false; }

  // Consumers.

  StatusOr<int> ConsumeInt(TSpan *tspan);

  StatusOr<double> ConsumeDouble(TSpan *tspan);

  StatusOr<absl::variant<double, int>> ConsumeNumeric(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });
    PrintAttempt("NUMERIC");

    return AnyVariant<double, int>(
        absl::bind_front(&Parser::ConsumeDouble, this),
        absl::bind_front(&Parser::ConsumeInt, this))(tspan);
  }

  // TODO(ambuc): This could return a string view into the underlying
  // string.
  StatusOr<std::string> ConsumeString(TSpan *tspan);

  StatusOr<int> Consume2Digit(TSpan *tspan);

  StatusOr<int> Consume4Digit(TSpan *tspan);

  StatusOr<Money::Currency> ConsumeCurrency(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    PrintAttempt("CURRENCY");
    return Any<Money::Currency>({
        absl::bind_front(&Parser::ConsumeCurrencySymbol, this),
        absl::bind_front(&Parser::ConsumeCurrencyWord, this),
    })(tspan);
  }

  StatusOr<Money> ConsumeMoney(TSpan *tspan);
  StatusOr<absl::Time> ConsumeDateTime(TSpan *tspan);
  StatusOr<absl::TimeZone> ConsumeTimeOffset(TSpan *tspan);

  StatusOr<Amount> ConsumeAmount(TSpan *tspan);

  StatusOr<PointLocation> ConsumePointLocation(TSpan *tspan);

  StatusOr<RangeLocation> ConsumeRangeLocation(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    PrintAttempt("RANGE_LOCATION");
    return Any<RangeLocation>({
        absl::bind_front(&Parser::ConsumeRangeLocationPointThenAny, this),
        absl::bind_front(&Parser::ConsumeRangeLocationRowThenRow, this),
        absl::bind_front(&Parser::ConsumeRangeLocationColThenCol, this),
    })(tspan);
  }

  // ( .* )
  StatusOr<std::vector<Expression>> ConsumeParentheses(TSpan *tspan);

  // [A-Z0-9_]
  StatusOr<std::string> ConsumeFnName(TSpan *tspan);

  StatusOr<Expression> ConsumeExpression(TSpan *tspan);

private:
  Options options_;

  int depth_{0};

  // Logging w/ depth_
  void PrintAttempt(const std::string &step) {
    if (options_.should_log_verbosely) {
      std::cout << std::string(depth_, ' ') << step << std::endl;
    }
  }
  void PrintStep(TSpan *lcl, TSpan *tspan, const std::string &step) {
    if (options_.should_log_verbosely) {
      auto whole = PrintTSpan(tspan);
      auto remaining = PrintTSpan(lcl);
      std::cout << absl::StreamFormat(
                       "%sParsed `%s` as an %s", std::string(depth_, ' '),
                       whole.substr(0, whole.size() - remaining.size()), step)
                << std::endl;
    }
  }

  // RepeatGuard apparatus
  using CacheItem = std::tuple<std::string, TSpan::pointer, TSpan::size_type>;
  absl::flat_hash_set<CacheItem> cache_;

  // NB: RepeatGuards are only necessary for right-recursive expressions... I
  // think.
  Status RepeatGuard(std::string step, TSpan *tspan) {
    CacheItem item = {step, tspan->data(), tspan->size()};

    if (cache_.contains(item)) {
      return Status(::google::protobuf::util::error::INVALID_ARGUMENT,
                    "RepeatGuard denied! Already been here");
    }
    cache_.insert(item);

    return OkStatus();
  }

  // Private consumers.

  // Consumes the token |type| off |tspan| and returns the token's held
  // |.value|.
  StatusOr<std::string_view> ConsumeExact(Token::T type, TSpan *tspan);

  StatusOr<Money::Currency> ConsumeCurrencyWord(TSpan *tspan);
  StatusOr<Money::Currency> ConsumeCurrencySymbol(TSpan *tspan);

  StatusOr<int> ConsumeRowIndicator(TSpan *tspan);
  StatusOr<int> ConsumeColIndicator(TSpan *tspan);

  StatusOr<RangeLocation> ConsumeRangeLocationPointThenAny(TSpan *tspan);
  StatusOr<RangeLocation> ConsumeRangeLocationRowThenRow(TSpan *tspan);
  StatusOr<RangeLocation> ConsumeRangeLocationColThenCol(TSpan *tspan);

  StatusOr<int> ConsumeDateFullYear(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    PrintAttempt("FULL_YEAR");
    return Consume4Digit(tspan);
  }

  StatusOr<int> ConsumeDateMonth(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    static auto r = [](int i) { return 1 <= i && i <= 12; };
    PrintAttempt("DATE_MONTH");
    return WithRestriction<int>(r)(
        absl::bind_front(&Parser::Consume2Digit, this))(tspan);
  }

  StatusOr<int> ConsumeDateMDay(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    static auto r = [](int i) { return 1 <= i && i <= 31; };
    PrintAttempt("DATE_MDAY");
    return WithRestriction<int>(r)(
        absl::bind_front(&Parser::Consume2Digit, this))(tspan);
  }

  StatusOr<int> ConsumeTimeHour(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    static auto r = [](int i) { return 0 <= i && i <= 23; };
    PrintAttempt("TIME_HOUR");
    return WithRestriction<int>(r)(
        absl::bind_front(&Parser::Consume2Digit, this))(tspan);
  }

  StatusOr<int> ConsumeTimeMinute(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    static auto r = [](int i) { return 0 <= i && i <= 59; };
    PrintAttempt("TIME_MINUTE");
    return WithRestriction<int>(r)(
        absl::bind_front(&Parser::Consume2Digit, this))(tspan);
  }
  StatusOr<int> ConsumeTimeSecond(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    // Up to 60, counting leap seconds.
    static auto r = [](int i) { return 0 <= i && i <= 60; };
    PrintAttempt("TIME_SECOND");
    return WithRestriction<int>(r)(
        absl::bind_front(&Parser::Consume2Digit, this))(tspan);
  }
  StatusOr<double> ConsumeTimeSecFrac(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    PrintAttempt("TIME_SEC_FRAC");
    return ConsumeDouble(tspan);
  }

  StatusOr<Expression::OpUnary> ConsumeOpUnaryText(TSpan *tspan);

  StatusOr<Expression::OpUnary> ConsumeOpUnary(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    PrintAttempt("OP_UNARY");
    return Any<Expression::OpUnary>({
        absl::bind_front(&Parser::ConsumeOpUnaryText, this),
    })(tspan);
  }

  StatusOr<Expression::OpBinary> ConsumeOpBinaryText(TSpan *tspan);

  // Consumes "+", "-", "/", "*", "%", etc. and returns the string version for
  // prefix notation.
  StatusOr<std::string> ConsumeOpBinaryInfixFn(TSpan *tspan);

  StatusOr<Expression::OpBinary> ConsumeOpBinaryInfix(TSpan *tspan);

  StatusOr<Expression::OpBinary> ConsumeOpBinary(TSpan *tspan) {
    depth_++;
    auto d = MakeCleanup([&] { depth_--; });

    return Any<Expression::OpBinary>({
        absl::bind_front(&Parser::ConsumeOpBinaryText, this),
        absl::bind_front(&Parser::ConsumeOpBinaryInfix, this),
    })(tspan);
  }

  StatusOr<Expression::OpTernary> ConsumeOpTernary(TSpan *tspan);
};

} // namespace formula
} // namespace latis

#endif // SRC_PARSER_PARSER_H_
