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
  ::google::protobuf::util::StatusOr<int> ConsumeInt(TSpan *tspan);

  ::google::protobuf::util::StatusOr<double> ConsumeDouble(TSpan *tspan);
  ::google::protobuf::util::StatusOr<absl::variant<double, int>>
  ConsumeNumeric(TSpan *tspan);

  // TODO(ambuc): This could return a string view into the underlying
  // string.
  ::google::protobuf::util::StatusOr<std::string_view>
  ConsumeString(TSpan *tspan);

  ::google::protobuf::util::StatusOr<int> Consume2Digit(TSpan *tspan);
  ::google::protobuf::util::StatusOr<int> Consume4Digit(TSpan *tspan);

  ::google::protobuf::util::StatusOr<Money::Currency>
  ConsumeCurrency(TSpan *tspan);

  ::google::protobuf::util::StatusOr<Money> ConsumeMoney(TSpan *tspan);
  ::google::protobuf::util::StatusOr<bool> ConsumeBool(TSpan *tspan);

  ::google::protobuf::util::StatusOr<absl::Time> ConsumeDateTime(TSpan *tspan);
  ::google::protobuf::util::StatusOr<absl::TimeZone>
  ConsumeTimeOffset(TSpan *tspan);

  ::google::protobuf::util::StatusOr<Amount> ConsumeAmount(TSpan *tspan);

  ::google::protobuf::util::StatusOr<PointLocation>
  ConsumePointLocation(TSpan *tspan);
  ::google::protobuf::util::StatusOr<RangeLocation>
  ConsumeRangeLocation(TSpan *tspan);

  ::google::protobuf::util::StatusOr<std::vector<Expression>>
  ConsumeParentheses(TSpan *tspan);

  ::google::protobuf::util::StatusOr<std::string> ConsumeFnName(TSpan *tspan);

  ::google::protobuf::util::StatusOr<Expression>
  ConsumeExpression(TSpan *tspan);

private:
  Options options_;

  int depth_{0};

  // Logging w/ depth_
  void PrintAttempt(TSpan *tspan, const std::string &step) {
    if (options_.should_log_verbosely) {
      std::cout << depth_ << "\t" << PrintTSpan(tspan)
                << std::string(2 * depth_, ' ') << step << std::endl;
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
  using CacheItem = std::tuple<std::string, TSpan::pointer>;
  using Cache = absl::flat_hash_set<CacheItem>;
  Cache cache_;

  // NB: RepeatGuards are only necessary for right-recursive expressions... I
  // think.
  ::google::protobuf::util::StatusOr<Cache::key_type>
  RepeatGuard(std::string step, TSpan *tspan) {
    CacheItem item = {step, tspan->data()};

    if (cache_.contains(item)) {
      return ::google::protobuf::util::Status(
          ::google::protobuf::util::error::INVALID_ARGUMENT,
          "RepeatGuard denied! Already been here");
    }
    cache_.insert(item);
    return item;
  }

  // Private consumers.

  ::google::protobuf::util::StatusOr<std::string_view>
  ConsumeExact(Token::T type, TSpan *tspan);

  ::google::protobuf::util::StatusOr<Money::Currency>
  ConsumeCurrencyWord(TSpan *tspan);
  ::google::protobuf::util::StatusOr<Money::Currency>
  ConsumeCurrencySymbol(TSpan *tspan);

  ::google::protobuf::util::StatusOr<int> ConsumeRowIndicator(TSpan *tspan);
  ::google::protobuf::util::StatusOr<int> ConsumeColIndicator(TSpan *tspan);

  ::google::protobuf::util::StatusOr<RangeLocation>
  ConsumeRangeLocationPointThenAny(TSpan *tspan);
  ::google::protobuf::util::StatusOr<RangeLocation>
  ConsumeRangeLocationRowThenRow(TSpan *tspan);
  ::google::protobuf::util::StatusOr<RangeLocation>
  ConsumeRangeLocationColThenCol(TSpan *tspan);

  ::google::protobuf::util::StatusOr<int> ConsumeDateFullYear(TSpan *tspan);
  ::google::protobuf::util::StatusOr<int> ConsumeDateMonth(TSpan *tspan);
  ::google::protobuf::util::StatusOr<int> ConsumeDateMDay(TSpan *tspan);
  ::google::protobuf::util::StatusOr<int> ConsumeTimeHour(TSpan *tspan);
  ::google::protobuf::util::StatusOr<int> ConsumeTimeMinute(TSpan *tspan);
  ::google::protobuf::util::StatusOr<int> ConsumeTimeSecond(TSpan *tspan);
  ::google::protobuf::util::StatusOr<double> ConsumeTimeSecFrac(TSpan *tspan);

  // Consumes "+", "-", "/", "*", "%", etc. and returns the string version for
  // prefix notation.
  ::google::protobuf::util::StatusOr<std::string_view>
  ConsumeOpBinaryInfixFn(TSpan *tspan);
  ::google::protobuf::util::StatusOr<Expression::Operation>
  ConsumeOperationInfix(TSpan *tspan);
  ::google::protobuf::util::StatusOr<Expression::Operation>
  ConsumeOperationPrefix(TSpan *tspan);

  ::google::protobuf::util::StatusOr<Expression::Operation>
  ConsumeOperation(TSpan *tspan);
};

} // namespace formula
} // namespace latis

#endif // SRC_PARSER_PARSER_H_
