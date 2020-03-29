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

#include "src/formula/parser.h"

#include "src/formula/parser_combinators.h"
#include "src/utils/status_macros.h"
#include "src/xy.h"

#include "absl/functional/bind_front.h"
#include "absl/memory/memory.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"

namespace latis {
namespace formula {

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;
using ::google::protobuf::util::error::OK;

namespace {

// https://www.bfilipek.com/2018/09/visit-variants.html
template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...)->overload<Ts...>;

// If given a tspan where the -1st token is lparen, will return the index of the
// matching parenthesis. Will return
//
//                    0123456789
// MatchParentheses("(2),Baz(3)") == 2
// MatchParentheses("(Foo(1)),Baz(3)") == 7
StatusOr<TSpan::iterator> MatchParentheses(TSpan *tspan) {
  if (tspan->empty()) {
    return Status(INVALID_ARGUMENT, "MatchParentheses failed, empty");
  }
  int depth = 1;
  for (TSpan::iterator it = tspan->begin(); it != tspan->end(); ++it) {
    if (it->type == Token::T::lparen) {
      depth++;
    } else if (it->type == Token::T::rparen) {
      depth--;
    }
    if (depth == 0) {
      return it;
    }
  }
  return Status(INVALID_ARGUMENT, "MatchParentheses failed, no ')'.");
}

} // namespace

StatusOr<std::string_view> Parser::ConsumeExact(Token::T type, TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "EXACT");

  if (tspan->empty()) {
    return Status(
        INVALID_ARGUMENT,
        absl::StrFormat("Can't ConsumeExact (format %d): empty", type));
  }

  const auto front = tspan->front();

  if (front.type != type) {
    return Status(INVALID_ARGUMENT,
                  absl::StrFormat(
                      "Can't ConsumeExact: Wrong format: expected %d found %d",
                      type, front.type));
  }

  tspan->remove_prefix(1);
  return front.value;
}

StatusOr<int> Parser::ConsumeInt(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "INT");

  TSpan lcl = *tspan;

  std::string_view value;
  ASSIGN_OR_RETURN_(value, ConsumeExact(Token::T::numeric, &lcl));

  int resultant;
  if (!absl::SimpleAtoi(value, &resultant)) {
    return Status(INVALID_ARGUMENT, "Can't ConsumeInt: not a number");
  }

  PrintStep(&lcl, tspan, "INT");
  *tspan = lcl;
  return resultant;
}

StatusOr<double> Parser::ConsumeDouble(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "DOUBLE");

  TSpan lcl = *tspan;

  std::tuple<absl::optional<int>, std::string_view, absl::optional<int>> t;
  ASSIGN_OR_RETURN_(
      t,
      (InSequence<absl::optional<int>, std::string_view, absl::optional<int>>(
          // int
          Maybe<int>(absl::bind_front(&Parser::ConsumeInt, this)),
          // period
          absl::bind_front(&Parser::ConsumeExact, this, Token::T::period),
          // int
          Maybe<int>(absl::bind_front(&Parser::ConsumeInt, this)))(&lcl)));

  double resultant{0};
  if (const auto before = std::get<0>(t); before.has_value()) {
    resultant += static_cast<double>(before.value());
  }
  if (const auto after = std::get<2>(t); after.has_value()) {
    if (after != 0) {
      double after_mut = static_cast<double>(after.value());
      // 123 => .123.
      while (after_mut >= 1.0) {
        after_mut /= 10.0;
      }
      resultant += after_mut;
    }
  }

  PrintStep(&lcl, tspan, "DOUBLE");
  *tspan = lcl;
  return resultant;
}

StatusOr<absl::variant<double, int>> Parser::ConsumeNumeric(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "NUMERIC");

  return AnyVariant<double, int>(absl::bind_front(&Parser::ConsumeDouble, this),
                                 absl::bind_front(&Parser::ConsumeInt, this))(
      tspan);
}

StatusOr<std::string> Parser::ConsumeString(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "STRING");

  TSpan lcl = *tspan;
  std::string_view resultant;
  ASSIGN_OR_RETURN_(resultant, ConsumeExact(Token::T::quote, &lcl));

  PrintStep(&lcl, tspan, "STRING");
  *tspan = lcl;
  return std::string(resultant);
}

StatusOr<int> Parser::Consume2Digit(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "2DIGIT");

  TSpan lcl = *tspan;
  std::string_view value;
  ASSIGN_OR_RETURN_(value, ConsumeExact(Token::T::numeric, &lcl));

  if (value.size() != 2) {
    return Status(INVALID_ARGUMENT, "Can't Consume2Digit: not 2 digits");
  }

  int resultant;
  if (!absl::SimpleAtoi(value, &resultant)) {
    return Status(INVALID_ARGUMENT, "Can't Consume2Digit: not a number");
  }

  PrintStep(&lcl, tspan, "2DIGIT");
  *tspan = lcl;
  return resultant;
}

StatusOr<int> Parser::Consume4Digit(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "4DIGIT");

  TSpan lcl = *tspan;
  std::string_view value;
  ASSIGN_OR_RETURN_(value, ConsumeExact(Token::T::numeric, &lcl));

  if (value.size() != 4) {
    return Status(INVALID_ARGUMENT, "Can't Consume4Digit: not 4 digits");
  }

  int resultant;
  if (!absl::SimpleAtoi(value, &resultant)) {
    return Status(INVALID_ARGUMENT, "Can't Consume4Digit: not a number");
  }

  PrintStep(&lcl, tspan, "4DIGIT");
  *tspan = lcl;
  return resultant;
}

StatusOr<Money::Currency> Parser::ConsumeCurrency(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });

  PrintAttempt(tspan, "CURRENCY");
  return Any<Money::Currency>({
      absl::bind_front(&Parser::ConsumeCurrencySymbol, this),
      absl::bind_front(&Parser::ConsumeCurrencyWord, this),
  })(tspan);
}

// Expects "USD" or "CAD".
StatusOr<Money::Currency> Parser::ConsumeCurrencyWord(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "CURRENCY_WORD");

  static std::unordered_map<std::string, Money::Currency> lookup_map{
      {"USD", Money::USD}, {"CAD", Money::CAD}};

  return WithLookup(lookup_map)(
      absl::bind_front(&Parser::ConsumeExact, this, Token::T::alpha))(tspan);
}

StatusOr<Money::Currency> Parser::ConsumeCurrencySymbol(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "CURRENCY_SYMBOL");

  TSpan lcl = *tspan;

  if (!ConsumeExact(Token::T::dollar, &lcl).ok()) {
    return Status(INVALID_ARGUMENT,
                  "Can't ConsumeCurrencySymbol: no currency symbol");
  }

  PrintStep(&lcl, tspan, "CURRENCY_SYMBOL");
  *tspan = lcl;
  return Money::USD;
}

StatusOr<Money> Parser::ConsumeMoney(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "MONEY");

  TSpan lcl = *tspan;
  Money money;

  // Set currency
  Money::Currency currency{Money::UNKNOWN};
  ASSIGN_OR_RETURN_(currency, ConsumeCurrency(&lcl));
  money.set_currency(currency);

  // Set dollars and cents.
  absl::variant<double, int> numeric;
  ASSIGN_OR_RETURN_(numeric, ConsumeNumeric(&lcl));

  std::visit(overload{
                 [&money](int i) { money.set_dollars(i); },
                 [&money](double d) {
                   int dollars = static_cast<int>(floor(d));
                   money.set_dollars(dollars);
                   money.set_cents(
                       static_cast<int>(round((d - dollars) * 100.0)));
                 },
             },
             numeric);

  PrintStep(&lcl, tspan, "MONEY");
  *tspan = lcl;
  return money;
}

StatusOr<bool> Parser::ConsumeBool(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "BOOL");

  TSpan lcl = *tspan;
  bool resultant;

  std::string_view extracted;
  ASSIGN_OR_RETURN_(extracted, ConsumeExact(Token::T::alpha, &lcl));

  if (extracted == "True") {
    resultant = true;
  } else if (extracted == "False") {
    resultant = false;
  } else {
    return Status(INVALID_ARGUMENT, "neither 'True' nor 'False'.");
  }

  PrintStep(&lcl, tspan, "BOOL");
  *tspan = lcl;
  return resultant;
}

StatusOr<absl::TimeZone> Parser::ConsumeTimeOffset(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "TIME_OFFSET");

  TSpan lcl = *tspan;

  std::tuple<std::string_view, int, std::string_view, int> t;
  ASSIGN_OR_RETURN_(
      t,
      (InSequence<std::string_view, int, std::string_view, int>(
          // + / -
          Any<std::string_view>(
              {absl::bind_front(&Parser::ConsumeExact, this, Token::T::plus),
               absl::bind_front(&Parser::ConsumeExact, this, Token::T::minus)}),
          // TIME_HOUR
          absl::bind_front(&Parser::ConsumeTimeHour, this),
          // :
          absl::bind_front(&Parser::ConsumeExact, this, Token::T::colon),
          // TIME_MINUTE
          absl::bind_front(&Parser::ConsumeTimeMinute, this))(&lcl)));

  int posneg = std::get<0>(t) == "+" ? 1 : -1;
  int hour = std::get<1>(t) * 60 * 60;
  int min = std::get<3>(t) * 60;

  PrintStep(&lcl, tspan, "TIME_OFFSET");
  *tspan = lcl;
  return absl::FixedTimeZone(posneg * (hour + min));
}

StatusOr<absl::Time> Parser::ConsumeDateTime(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "DATE_TIME");

  auto only_T = [](std::string_view s) -> bool { return s == "T"; };

  TSpan lcl = *tspan;

  // DATE_FULLYEAR "-" DATE_MONTH "-" DATE_MDAY "T" TIME_HOUR ":" TIME_MINUTE
  // ":" TIME_SECOND [TIME_SECFRAC] TIME_OFFSET
  std::tuple<int, std::string_view, int, std::string_view, int,
             std::string_view, int, std::string_view, int, std::string_view,
             int, absl::optional<double>, absl::TimeZone>
      t;
  ASSIGN_OR_RETURN_(
      t, (InSequence<int, std::string_view, int, std::string_view, int,
                     std::string_view, int, std::string_view, int,
                     std::string_view, int, absl::optional<double>,
                     absl::TimeZone>(
             // DATE_FULLYEAR
             absl::bind_front(&Parser::ConsumeDateFullYear, this),
             // -
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::minus),
             // DATE_MONTH
             absl::bind_front(&Parser::ConsumeDateMonth, this),
             // -
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::minus),
             // DATE_MDAY
             absl::bind_front(&Parser::ConsumeDateMDay, this),
             // T
             WithRestriction<std::string_view>(only_T)(absl::bind_front(
                 &Parser::ConsumeExact, this, Token::T::alpha)),
             // TIME_HOUR
             absl::bind_front(&Parser::ConsumeTimeHour, this),
             // :
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::colon),
             // TIME_MINUTE
             absl::bind_front(&Parser::ConsumeTimeMinute, this),
             // :
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::colon),
             // TIME_SECOND
             absl::bind_front(&Parser::ConsumeTimeSecond, this),
             // [TIME_SECFRAC]
             Maybe<double>(absl::bind_front(&Parser::ConsumeTimeSecFrac, this)),
             // TIME_OFFSET
             absl::bind_front(&Parser::ConsumeTimeOffset, this))(&lcl)));

  // Tz-aligned.
  absl::Time resultant = absl::FromCivil(
      /*civil_second=*/
      absl::CivilSecond(
          /*year=*/std::get<0>(t), /*month=*/std::get<2>(t),
          /*day=*/std::get<4>(t),
          /*hour=*/std::get<6>(t), /*minute=*/std::get<8>(t),
          /*second=*/std::get<10>(t)),
      /*tz=*/std::get<12>(t));

  // If applicable,
  if (const auto secfrac = std::get<11>(t); secfrac.has_value()) {
    // ...add secfrac.
    resultant += absl::Milliseconds(round(secfrac.value() * 1000));
  }

  PrintStep(&lcl, tspan, "DATETIME");
  *tspan = lcl;
  return resultant;
}

StatusOr<Amount> Parser::ConsumeAmount(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "AMOUNT");

  TSpan lcl = *tspan;

  absl::variant<std::string, absl::Time, double, int, Money, bool> amount;

  ASSIGN_OR_RETURN_(
      amount, (AnyVariant<std::string, absl::Time, double, int, Money, bool>(
                  absl::bind_front(&Parser::ConsumeString, this),
                  absl::bind_front(&Parser::ConsumeDateTime, this),
                  absl::bind_front(&Parser::ConsumeDouble, this),
                  absl::bind_front(&Parser::ConsumeInt, this),
                  absl::bind_front(&Parser::ConsumeMoney, this),
                  absl::bind_front(&Parser::ConsumeBool, this))(&lcl)));

  Amount resultant;
  std::visit(
      overload{
          [&resultant](std::string s) { resultant.set_str_amount(s); },
          [&resultant](absl::Time t) {
            resultant.mutable_timestamp_amount()->set_seconds(
                absl::ToInt64Seconds(t - absl::UnixEpoch()));
            // TODO(ambuc) figure out microseconds.
            // See
            // https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/timestamp.proto#L133-L137.
          },
          [&resultant](double d) { resultant.set_double_amount(d); },
          [&resultant](int d) { resultant.set_int_amount(d); },
          [&resultant](Money m) { *resultant.mutable_money_amount() = m; },
          [&resultant](bool b) { resultant.set_bool_amount(b); },
      },
      amount);

  PrintStep(&lcl, tspan, "AMOUNT");
  *tspan = lcl;
  return resultant;
}

StatusOr<int> Parser::ConsumeRowIndicator(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "ROW_INDICATOR");

  auto tr = [](int i) -> int { return i - 1; };
  auto r = [](int i) -> bool { return i > 0; };
  return WithTransformation<int, int>(tr)(WithRestriction<int>(r)(
      absl::bind_front(&Parser::ConsumeInt, this)))(tspan);
}

StatusOr<int> Parser::ConsumeColIndicator(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "COL_INDICATOR");

  // TODO(ambuc): WithTransformationReturningOptional ?
  TSpan lcl = *tspan;

  const auto maybe_str_view = ConsumeExact(Token::T::alpha, &lcl);

  if (!maybe_str_view.ok()) {
    return Status(
        INVALID_ARGUMENT,
        "Can't ConsumeColIndicator: LOCATION must begin with 1*UPPERCASE.");
  }

  const auto maybe_int = XY::ColumnLetterToInteger(maybe_str_view.ValueOrDie());
  if (!maybe_int.ok()) {
    return Status(
        INVALID_ARGUMENT,
        "Can't ConsumeColIndicator: LOCATION must begin with 1*UPPERCASE.");
  }

  PrintStep(&lcl, tspan, "COL_INDICATOR");
  *tspan = lcl;
  return maybe_int.ValueOrDie();
}

StatusOr<PointLocation> Parser::ConsumePointLocation(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "POINT_LOCATION");
  TSpan lcl = *tspan;
  PointLocation resultant;

  std::tuple<int, int> t;
  ASSIGN_OR_RETURN_(
      t, (InSequence<int, int>(
             // COL
             absl::bind_front(&Parser::ConsumeColIndicator, this),
             // ROW
             absl::bind_front(&Parser::ConsumeRowIndicator, this))(&lcl)));

  resultant.set_col(std::get<0>(t));
  resultant.set_row(std::get<1>(t));

  PrintStep(&lcl, tspan, "POINT_LOCATION");
  *tspan = lcl;
  return resultant;
}

StatusOr<RangeLocation> Parser::ConsumeRangeLocation(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });

  PrintAttempt(tspan, "RANGE_LOCATION");
  return Any<RangeLocation>({
      absl::bind_front(&Parser::ConsumeRangeLocationPointThenAny, this),
      absl::bind_front(&Parser::ConsumeRangeLocationRowThenRow, this),
      absl::bind_front(&Parser::ConsumeRangeLocationColThenCol, this),
  })(tspan);
}

StatusOr<RangeLocation> Parser::ConsumeRangeLocationPointThenAny(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "RANGE_LOCATION_POINT_THEN_ANY");
  TSpan lcl = *tspan;
  RangeLocation resultant;

  PointLocation pl;
  ASSIGN_OR_RETURN_(pl, ConsumePointLocation(&lcl));
  *resultant.mutable_from_cell() = pl;

  if (!ConsumeExact(Token::T::colon, &lcl).ok()) {
    return Status(INVALID_ARGUMENT, "Can't ConsumeRangeLocationPointThenAny: "
                                    "RANGE_LOCATION must have a ';'.");
  }

  if (const auto to_pl = ConsumePointLocation(&lcl); to_pl.ok()) {
    *resultant.mutable_to_cell() = to_pl.ValueOrDie();
  } else if (const auto v = ConsumeRowIndicator(&lcl); v.ok()) {
    resultant.set_to_row(v.ValueOrDie());
  } else if (const auto c = ConsumeColIndicator(&lcl); c.ok()) {
    resultant.set_to_col(c.ValueOrDie());
  } else {
    return Status(INVALID_ARGUMENT,
                  "Can't ConsumeRangeLocationPointThenAny: RANGE_LOCATION must "
                  "end in a point/row/col.");
  }

  PrintStep(&lcl, tspan, "RANGE_LOCATION_POINT_THEN_ANY");
  *tspan = lcl;
  return resultant;
}

StatusOr<RangeLocation> Parser::ConsumeRangeLocationRowThenRow(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "RANGE_LOCATION_ROW_THEN_ROW");
  TSpan lcl = *tspan;
  RangeLocation resultant;

  std::tuple<int, std::string_view, int> t;
  ASSIGN_OR_RETURN_(
      t, (InSequence<int, std::string_view, int>(
             // ROW
             absl::bind_front(&Parser::ConsumeRowIndicator, this),
             // :
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::colon),
             // ROW
             absl::bind_front(&Parser::ConsumeRowIndicator, this))(&lcl)));
  resultant.set_from_row(std::get<0>(t));
  resultant.set_to_row(std::get<2>(t));

  PrintStep(&lcl, tspan, "RANGE_LOCATION_ROW_THEN_ROW");
  *tspan = lcl;
  return resultant;
}

StatusOr<RangeLocation> Parser::ConsumeRangeLocationColThenCol(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "RANGE_LOCATION_COL_THEN_COL");
  TSpan lcl = *tspan;
  RangeLocation resultant;

  std::tuple<int, std::string_view, int> t;
  ASSIGN_OR_RETURN_(
      t, (InSequence<int, std::string_view, int>(
             // COL,
             absl::bind_front(&Parser::ConsumeColIndicator, this),
             // :
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::colon),
             // COL
             absl::bind_front(&Parser::ConsumeColIndicator, this))(&lcl)));

  resultant.set_from_col(std::get<0>(t));
  resultant.set_to_col(std::get<2>(t));

  PrintStep(&lcl, tspan, "RANGE_LOCATION_COL_THEN_COL");
  *tspan = lcl;
  return resultant;
}

StatusOr<int> Parser::ConsumeDateFullYear(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });

  PrintAttempt(tspan, "FULL_YEAR");
  return Consume4Digit(tspan);
}

StatusOr<int> Parser::ConsumeDateMonth(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });

  static auto r = [](int i) { return 1 <= i && i <= 12; };
  PrintAttempt(tspan, "DATE_MONTH");
  return WithRestriction<int>(r)(
      absl::bind_front(&Parser::Consume2Digit, this))(tspan);
}

StatusOr<int> Parser::ConsumeDateMDay(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });

  static auto r = [](int i) { return 1 <= i && i <= 31; };
  PrintAttempt(tspan, "DATE_MDAY");
  return WithRestriction<int>(r)(
      absl::bind_front(&Parser::Consume2Digit, this))(tspan);
}

StatusOr<int> Parser::ConsumeTimeHour(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });

  static auto r = [](int i) { return 0 <= i && i <= 23; };
  PrintAttempt(tspan, "TIME_HOUR");
  return WithRestriction<int>(r)(
      absl::bind_front(&Parser::Consume2Digit, this))(tspan);
}

StatusOr<int> Parser::ConsumeTimeMinute(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });

  static auto r = [](int i) { return 0 <= i && i <= 59; };
  PrintAttempt(tspan, "TIME_MINUTE");
  return WithRestriction<int>(r)(
      absl::bind_front(&Parser::Consume2Digit, this))(tspan);
}

StatusOr<int> Parser::ConsumeTimeSecond(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });

  // Up to 60, counting leap seconds.
  static auto r = [](int i) { return 0 <= i && i <= 60; };
  PrintAttempt(tspan, "TIME_SECOND");
  return WithRestriction<int>(r)(
      absl::bind_front(&Parser::Consume2Digit, this))(tspan);
}

StatusOr<double> Parser::ConsumeTimeSecFrac(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });

  PrintAttempt(tspan, "TIME_SEC_FRAC");
  return ConsumeDouble(tspan);
}

// [A-Z0-9_]
// TODO(ambuc): This could return a std::str_view into the underlying tspan.
StatusOr<std::string> Parser::ConsumeFnName(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "FN_NAME");
  TSpan lcl = *tspan;
  std::string resultant;

  while (true) {
    if (const auto maybe_alpha = ConsumeExact(Token::T::alpha, &lcl);
        maybe_alpha.ok()) {
      resultant += maybe_alpha.ValueOrDie();
    } else if (const auto maybe_int = ConsumeInt(&lcl); maybe_int.ok()) {
      resultant += std::to_string(maybe_int.ValueOrDie());
    } else if (ConsumeExact(Token::T::underscore, &lcl).ok()) {
      resultant += "_";
    } else {
      break;
    }
  }

  if (resultant.empty()) {
    return Status(INVALID_ARGUMENT,
                  "Can't ConsumeFnName: Can't have an empty fn name.");
  }

  if (std::any_of(resultant.cbegin(), resultant.cend(), islower)) {
    return Status(
        INVALID_ARGUMENT,
        "Can't ConsumeFnName: Can't have a fn name with lowercase letters.");
  }

  if (resultant.front() == '_') {
    return Status(INVALID_ARGUMENT, "Can't ConsumeFnName: Can't have a fn name "
                                    "which begins with an underscore");
  }

  if (isdigit(resultant.front())) {
    return Status(
        INVALID_ARGUMENT,
        "Can't ConsumeFnName: Can't have a fn name which begins with a digit.");
  }

  PrintStep(&lcl, tspan, "FN_NAME");
  *tspan = lcl;
  return resultant;
}

StatusOr<Expression::Operation> Parser::ConsumeOperationPrefix(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "OPERATION_PREFIX");
  TSpan lcl = *tspan;

  std::tuple<std::string, std::vector<Expression>> t;
  ASSIGN_OR_RETURN_(
      t, (InSequence<std::string, std::vector<Expression>>(
             // FN
             absl::bind_front(&Parser::ConsumeFnName, this),
             // PARENS
             absl::bind_front(&Parser::ConsumeParentheses, this))(&lcl)));

  Expression::Operation resultant;
  resultant.set_fn_name(std::get<0>(t));
  for (const Expression &expr : std::get<1>(t)) {
    *resultant.add_terms() = expr;
  }

  PrintStep(&lcl, tspan, "OPERATION_PREFIX");
  *tspan = lcl;
  return resultant;
}

StatusOr<std::string> Parser::ConsumeOpBinaryInfixFn(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "OP_BINARY_INFIX_FN");
  TSpan lcl = *tspan;
  std::string resultant;

  if (ConsumeExact(Token::T::plus, &lcl).ok()) {
    resultant = functions::kPLUS;
  } else if (ConsumeExact(Token::T::minus, &lcl).ok()) {
    resultant = functions::kMINUS;
  } else if (ConsumeExact(Token::T::asterisk, &lcl).ok()) {
    resultant = functions::kTIMES;
  } else if (ConsumeExact(Token::T::slash, &lcl).ok()) {
    resultant = functions::kDIVIDED_BY;
  } else if (ConsumeExact(Token::T::carat, &lcl).ok()) {
    resultant = functions::kPOW;
  } else if (ConsumeExact(Token::T::percent, &lcl).ok()) {
    resultant = functions::kMOD;
  } else if (ConsumeExact(Token::T::lthan, &lcl).ok()) {
    resultant = functions::kLTHAN;
  } else if (ConsumeExact(Token::T::gthan, &lcl).ok()) {
    resultant = functions::kGTHAN;
  } else if (InSequence<std::string, std::string>(
                 absl::bind_front(&Parser::ConsumeExact, this,
                                  Token::T::ampersand),
                 absl::bind_front(&Parser::ConsumeExact, this,
                                  Token::T::ampersand))(&lcl)
                 .ok()) {
    resultant = functions::kAND;
  } else if (InSequence<std::string, std::string>(
                 absl::bind_front(&Parser::ConsumeExact, this, Token::T::pipe),
                 absl::bind_front(&Parser::ConsumeExact, this, Token::T::pipe))(
                 &lcl)
                 .ok()) {
    resultant = functions::kOR;
  } else if (InSequence<std::string, std::string>(
                 absl::bind_front(&Parser::ConsumeExact, this, Token::T::lthan),
                 absl::bind_front(&Parser::ConsumeExact, this,
                                  Token::T::equals))(&lcl)
                 .ok()) {
    resultant = functions::kLEQ;
  } else if (InSequence<std::string, std::string>(
                 absl::bind_front(&Parser::ConsumeExact, this, Token::T::gthan),
                 absl::bind_front(&Parser::ConsumeExact, this,
                                  Token::T::equals))(&lcl)
                 .ok()) {
    resultant = functions::kGEQ;
  } else if (InSequence<std::string, std::string>(
                 absl::bind_front(&Parser::ConsumeExact, this,
                                  Token::T::equals),
                 absl::bind_front(&Parser::ConsumeExact, this,
                                  Token::T::equals))(&lcl)
                 .ok()) {
    resultant = functions::kEQ;
  } else if (InSequence<std::string, std::string>(
                 absl::bind_front(&Parser::ConsumeExact, this, Token::T::bang),
                 absl::bind_front(&Parser::ConsumeExact, this,
                                  Token::T::equals))(&lcl)
                 .ok()) {
    resultant = functions::kNEQ;
  } else {
    return Status(INVALID_ARGUMENT,
                  "Can't ConsumeOpBinaryInfixFn: Not a binary infix.");
  }

  PrintStep(&lcl, tspan, "OP_BINARY_INFIX_FN");
  *tspan = lcl;
  return resultant;
}

StatusOr<Expression::Operation> Parser::ConsumeOperationInfix(TSpan *tspan) {
  // Mark this point as tried. When this method ends, pop this key off the
  // stack.
  Cache::key_type key;
  ASSIGN_OR_RETURN_(key, RepeatGuard("consume_op_binary_infix", tspan));
  auto d2 = MakeCleanup([&] { cache_.erase(key); });

  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "OP_BINARY_INFIX");
  TSpan lcl = *tspan;

  std::tuple<Expression, std::string, Expression> t;
  ASSIGN_OR_RETURN_(
      t, (InSequence<Expression, std::string, Expression>(
             // EXPR
             absl::bind_front(&Parser::ConsumeExpression, this),
             // + - etc
             absl::bind_front(&Parser::ConsumeOpBinaryInfixFn, this),
             // EXPR
             absl::bind_front(&Parser::ConsumeExpression, this))(&lcl)));

  Expression::Operation resultant;
  *resultant.add_terms() = std::get<0>(t);
  resultant.set_fn_name(std::get<1>(t));
  *resultant.add_terms() = std::get<2>(t);

  PrintStep(&lcl, tspan, "OP_BINARY_INFIX");
  *tspan = lcl;
  return resultant;
}

StatusOr<Expression::Operation> Parser::ConsumeOperation(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "OPERATION");
  return Any<Expression::Operation>({
      absl::bind_front(&Parser::ConsumeOperationInfix, this),
      absl::bind_front(&Parser::ConsumeOperationPrefix, this),
  })(tspan);
}

StatusOr<std::vector<Expression>> Parser::ConsumeParentheses(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "PARENTHESES");

  TSpan lcl = *tspan;

  // check lcl[0] == '('
  if (!ConsumeExact(Token::T::lparen, &lcl).ok()) {
    return Status(INVALID_ARGUMENT, "Not a PARENTHESES, 1st char is not '('.");
  }

  TSpan::iterator rparen;
  ASSIGN_OR_RETURN_(rparen, MatchParentheses(&lcl));

  auto size_of_inner = std::distance(lcl.begin(), rparen);
  TSpan lcl_inner(lcl.data(), size_of_inner);

  std::vector<Expression> resultant{};

  // Pop Expressions off the inner stack.
  while (true) {
    Expression expr;
    ASSIGN_OR_RETURN_(expr, ConsumeExpression(&lcl_inner));
    resultant.push_back(expr);
    if (!ConsumeExact(Token::T::comma, &lcl_inner).ok()) {
      break;
    }
  }
  lcl.remove_prefix(size_of_inner + 1);

  PrintStep(&lcl, tspan, "PARENTHESES");
  *tspan = lcl;
  return resultant;
}

StatusOr<Expression> Parser::ConsumeExpression(TSpan *tspan) {
  depth_++;
  auto d = MakeCleanup([&] { depth_--; });
  PrintAttempt(tspan, "EXPRESSION");

  TSpan lcl = *tspan;
  Expression resultant;

  absl::variant<Expression::Operation, std::vector<Expression>, RangeLocation,
                PointLocation, Amount>
      expression;

  ASSIGN_OR_RETURN_(
      expression,
      (AnyVariant<Expression::Operation, std::vector<Expression>, RangeLocation,
                  PointLocation, Amount>(
          absl::bind_front(&Parser::ConsumeOperation, this),
          WithRestriction<std::vector<Expression>>(
              [](const std::vector<Expression> exprs) -> bool {
                return exprs.size() == 1;
              })(absl::bind_front(&Parser::ConsumeParentheses, this)),
          absl::bind_front(&Parser::ConsumeRangeLocation, this),
          absl::bind_front(&Parser::ConsumePointLocation, this),
          absl::bind_front(&Parser::ConsumeAmount, this))(&lcl)));

  std::visit(
      overload{
          [&resultant](std::vector<Expression> exprs) {
            // For (AMOUNT).
            assert(exprs.size() == 1);
            resultant = exprs[0];
          },
          [&resultant](Expression::Operation o) {
            *resultant.mutable_operation() = o;
          },
          [&resultant](RangeLocation rl) { *resultant.mutable_range() = rl; },
          [&resultant](PointLocation pl) { *resultant.mutable_lookup() = pl; },
          [&resultant](Amount value) { *resultant.mutable_value() = value; },
      },
      expression);

  PrintStep(&lcl, tspan, "EXPRESSION");
  *tspan = lcl;
  return resultant;
}

} // namespace formula
} //  namespace latis
