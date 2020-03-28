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
#include "src/status_utils/status_macros.h"
#include "src/xy.h"

#include "absl/functional/bind_front.h"
#include "absl/memory/memory.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"

namespace latis {
namespace formula {

namespace {

// https://www.bfilipek.com/2018/09/visit-variants.html
template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...)->overload<Ts...>;

} // namespace

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;
using ::google::protobuf::util::error::OK;

StatusOr<std::string_view> Parser::ConsumeExact(Token::T type, TSpan *tspan) {
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

  TSpan lcl = *tspan;

  std::string_view value;
  ASSIGN_OR_RETURN_(value, ConsumeExact(Token::T::numeric, &lcl));

  if (int resultant; !absl::SimpleAtoi(value, &resultant)) {
    return Status(INVALID_ARGUMENT, "Can't ConsumeInt: not a number");
  } else {
    PrintStep(tspan, "INT");
    *tspan = lcl;
    return resultant;
  }
}

StatusOr<double> Parser::ConsumeDouble(TSpan *tspan) {
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
    resultant += static_cast<double>(after.value() /
                                     pow(10.0, ceil(log10(after.value()))));
  }

  PrintStep(tspan, "DOUBLE");
  *tspan = lcl;
  return resultant;
}

StatusOr<std::string> Parser::ConsumeString(TSpan *tspan) {
  TSpan lcl = *tspan;
  std::string_view resultant;
  ASSIGN_OR_RETURN_(resultant, ConsumeExact(Token::T::quote, &lcl));

  PrintStep(tspan, "STRING");
  *tspan = lcl;
  return std::string(resultant);
}

StatusOr<int> Parser::Consume2Digit(TSpan *tspan) {
  TSpan lcl = *tspan;
  std::string_view value;
  ASSIGN_OR_RETURN_(value, ConsumeExact(Token::T::numeric, &lcl));

  if (value.size() != 2) {
    return Status(INVALID_ARGUMENT, "Can't Consume2Digit: not 2 digits");
  }

  if (int resultant; absl::SimpleAtoi(value, &resultant)) {
    PrintStep(tspan, "2DIGIT");
    *tspan = lcl;
    return resultant;
  }
  return Status(INVALID_ARGUMENT, "Can't Consume2Digit: not a number");
}

StatusOr<int> Parser::Consume4Digit(TSpan *tspan) {
  TSpan lcl = *tspan;
  std::string_view value;
  ASSIGN_OR_RETURN_(value, ConsumeExact(Token::T::numeric, &lcl));

  if (value.size() != 4) {
    return Status(INVALID_ARGUMENT, "Can't Consume4Digit: not 4 digits");
  }

  if (int resultant; absl::SimpleAtoi(value, &resultant)) {
    PrintStep(tspan, "4DIGIT");
    *tspan = lcl;
    return resultant;
  }
  return Status(INVALID_ARGUMENT, "Can't Consume4Digit: not a number");
}

// Expects "USD" or "CAD".
StatusOr<Money::Currency> Parser::ConsumeCurrencyWord(TSpan *tspan) {
  static std::unordered_map<std::string, Money::Currency> lookup_map{
      {"USD", Money::USD}, {"CAD", Money::CAD}};

  return WithLookup(lookup_map)(
      absl::bind_front(&Parser::ConsumeExact, this, Token::T::alpha))(tspan);
}

StatusOr<Money::Currency> Parser::ConsumeCurrencySymbol(TSpan *tspan) {
  if (TSpan lcl = *tspan; ConsumeExact(Token::T::dollar, &lcl).ok()) {
    PrintStep(tspan, "CURRENCY_SYMBOL");
    *tspan = lcl;
    return Money::USD;
  }
  return Status(INVALID_ARGUMENT,
                "Can't ConsumeCurrencySymbol: no currency symbol");
}

StatusOr<Money> Parser::ConsumeMoney(TSpan *tspan) {
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
                   int dollars = floor(d);
                   int cents = (d * 100) - (dollars * 100);
                   money.set_dollars(dollars);
                   money.set_cents(cents);
                 },
             },
             numeric);

  PrintStep(tspan, "MONEY");
  *tspan = lcl;
  return money;
}

StatusOr<absl::TimeZone> Parser::ConsumeTimeOffset(TSpan *tspan) {
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

  PrintStep(tspan, "TIME_OFFSET");
  *tspan = lcl;
  return absl::FixedTimeZone(posneg * (hour + min));
}

StatusOr<absl::Time> Parser::ConsumeDateTime(TSpan *tspan) {
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

  PrintStep(tspan, "DATETIME");
  *tspan = lcl;
  return resultant;
}

StatusOr<Amount> Parser::ConsumeAmount(TSpan *tspan) {
  TSpan lcl = *tspan;

  absl::variant<std::string, absl::Time, double, int, Money> amount;

  ASSIGN_OR_RETURN_(amount,
                    (AnyVariant<std::string, absl::Time, double, int, Money>(
                        absl::bind_front(&Parser::ConsumeString, this),
                        absl::bind_front(&Parser::ConsumeDateTime, this),
                        absl::bind_front(&Parser::ConsumeDouble, this),
                        absl::bind_front(&Parser::ConsumeInt, this),
                        absl::bind_front(&Parser::ConsumeMoney, this))(&lcl)));

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
          [&resultant](Money m) { *resultant.mutable_money_amount() = m; },
          [&resultant](double d) { resultant.set_double_amount(d); },
          [&resultant](int i) { resultant.set_int_amount(i); },
      },
      amount);

  PrintStep(tspan, "AMOUNT");
  *tspan = lcl;
  return resultant;
}

StatusOr<int> Parser::ConsumeRowIndicator(TSpan *tspan) {
  auto tr = [](int i) -> int { return i - 1; };
  auto r = [](int i) -> bool { return i > 0; };
  return WithTransformation<int, int>(tr)(WithRestriction<int>(r)(
      absl::bind_front(&Parser::ConsumeInt, this)))(tspan);
}

StatusOr<int> Parser::ConsumeColIndicator(TSpan *tspan) {
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

  PrintStep(tspan, "COL_INDICATOR");
  *tspan = lcl;
  return maybe_int.ValueOrDie();
}

StatusOr<PointLocation> Parser::ConsumePointLocation(TSpan *tspan) {
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

  PrintStep(tspan, "POINT_LOCATION");
  *tspan = lcl;
  return resultant;
}

StatusOr<RangeLocation> Parser::ConsumeRangeLocationPointThenAny(TSpan *tspan) {
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

  PrintStep(tspan, "RANGE_LOCATION_POINT_THEN_ANY");
  *tspan = lcl;
  return resultant;
}

StatusOr<RangeLocation> Parser::ConsumeRangeLocationRowThenRow(TSpan *tspan) {
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

  PrintStep(tspan, "RANGE_LOCATION_ROW_THEN_ROW");
  *tspan = lcl;
  return resultant;
}

StatusOr<RangeLocation> Parser::ConsumeRangeLocationColThenCol(TSpan *tspan) {
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

  PrintStep(tspan, "RANGE_LOCATION_COL_THEN_COL");
  *tspan = lcl;
  return resultant;
}

// [A-Z0-9_]
// TODO(ambuc): This could return a std::str_view into the underlying tspan.
StatusOr<std::string> Parser::ConsumeFnName(TSpan *tspan) {
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

  PrintStep(tspan, "FN_NAME");
  *tspan = lcl;
  return resultant;
}

StatusOr<Expression::OpUnary> Parser::ConsumeOpUnaryText(TSpan *tspan) {
  TSpan lcl = *tspan;

  std::tuple<std::string, std::string_view, Expression, std::string_view> t;
  ASSIGN_OR_RETURN_(
      t,
      (InSequence<std::string, std::string_view, Expression, std::string_view>(
          // FN
          absl::bind_front(&Parser::ConsumeFnName, this),
          // (
          absl::bind_front(&Parser::ConsumeExact, this, Token::T::lparen),
          // EXPR
          absl::bind_front(&Parser::ConsumeExpression, this),
          // )
          absl::bind_front(&Parser::ConsumeExact, this, Token::T::rparen))(
          &lcl)));

  Expression::OpUnary resultant;
  resultant.set_operation(std::get<0>(t));
  *resultant.mutable_term1() = std::get<2>(t);

  PrintStep(tspan, "OP_UNARY_TEXT");
  *tspan = lcl;
  return resultant;
}

StatusOr<Expression::OpBinary> Parser::ConsumeOpBinaryText(TSpan *tspan) {
  TSpan lcl = *tspan;

  std::tuple<std::string, std::string_view, Expression, std::string_view,
             Expression, std::string_view>
      t;
  ASSIGN_OR_RETURN_(
      t, (InSequence<std::string, std::string_view, Expression,
                     std::string_view, Expression, std::string_view>(
             // FN
             absl::bind_front(&Parser::ConsumeFnName, this),
             // (
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::lparen),
             // EXPR
             absl::bind_front(&Parser::ConsumeExpression, this),
             // ,
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::comma),
             // EXPR
             absl::bind_front(&Parser::ConsumeExpression, this),
             // )
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::rparen))(
             &lcl)));

  Expression::OpBinary resultant;
  resultant.set_operation(std::get<0>(t));
  *resultant.mutable_term1() = std::get<2>(t);
  *resultant.mutable_term2() = std::get<4>(t);

  PrintStep(tspan, "OP_BINARY_TEXT");
  *tspan = lcl;
  return resultant;
}

StatusOr<std::string> Parser::ConsumeOpBinaryInfixFn(TSpan *tspan) {
  TSpan lcl = *tspan;
  std::string resultant;

  if (ConsumeExact(Token::T::plus, &lcl).ok()) {
    resultant = "PLUS";
  } else if (ConsumeExact(Token::T::minus, &lcl).ok()) {
    resultant = "MINUS";
  } else if (ConsumeExact(Token::T::asterisk, &lcl).ok()) {
    resultant = "TIMES";
  } else if (ConsumeExact(Token::T::slash, &lcl).ok()) {
    resultant = "DIVIDED_BY";
  } else if (ConsumeExact(Token::T::carat, &lcl).ok()) {
    resultant = "POW";
  } else if (ConsumeExact(Token::T::percent, &lcl).ok()) {
    resultant = "MOD";
  } else if (ConsumeExact(Token::T::lthan, &lcl).ok()) {
    resultant = "LTHAN";
  } else if (ConsumeExact(Token::T::gthan, &lcl).ok()) {
    resultant = "GTHAN";
  } else {
    return Status(INVALID_ARGUMENT,
                  "Can't ConsumeOpBinaryInfixFn: Not a binary infix.");
  }

  PrintStep(tspan, "OP_BINARY_INFIX_FN");
  *tspan = lcl;
  return resultant;
}

StatusOr<Expression::OpBinary> Parser::ConsumeOpBinaryInfix(TSpan *tspan) {
  // NB: RepeatGuards are only necessary for right-recursive expressions... I
  // think.
  RETURN_IF_ERROR_(RepeatGuard("consume_op_binary_infix", tspan));

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

  Expression::OpBinary resultant;
  *resultant.mutable_term1() = std::get<0>(t);
  resultant.set_operation(std::get<1>(t));
  *resultant.mutable_term2() = std::get<2>(t);

  PrintStep(tspan, "OP_BINARY_INFIX");
  *tspan = lcl;
  return resultant;
}

StatusOr<Expression> Parser::ConsumeParentheses(TSpan *tspan) {
  TSpan lcl = *tspan;
  std::tuple<std::string_view, Expression, std::string_view> t;
  ASSIGN_OR_RETURN_(
      t, (InSequence<std::string_view, Expression, std::string_view>(
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::lparen),
             absl::bind_front(&Parser::ConsumeExpression, this),
             absl::bind_front(&Parser::ConsumeExact, this, Token::T::rparen))(
             &lcl)));

  PrintStep(tspan, "PARENTHESES");
  *tspan = lcl;
  return std::get<1>(t);
}

StatusOr<Expression::OpTernary> Parser::ConsumeOpTernary(TSpan *tspan) {
  TSpan lcl = *tspan;

  std::tuple<std::string, std::string_view, Expression, std::string_view,
             Expression, std::string_view, Expression, std::string_view>
      t;
  ASSIGN_OR_RETURN_(
      t,
      (InSequence<std::string, std::string_view, Expression, std::string_view,
                  Expression, std::string_view, Expression, std::string_view>(
          // FN
          absl::bind_front(&Parser::ConsumeFnName, this),
          // (
          absl::bind_front(&Parser::ConsumeExact, this, Token::T::lparen),
          // EXPR
          absl::bind_front(&Parser::ConsumeExpression, this),
          // ,
          absl::bind_front(&Parser::ConsumeExact, this, Token::T::comma),
          // EXPR
          absl::bind_front(&Parser::ConsumeExpression, this),
          // ,
          absl::bind_front(&Parser::ConsumeExact, this, Token::T::comma),
          // EXPR
          absl::bind_front(&Parser::ConsumeExpression, this),
          // )
          absl::bind_front(&Parser::ConsumeExact, this, Token::T::rparen))(
          &lcl)));

  Expression::OpTernary resultant;
  resultant.set_operation(std::get<0>(t));
  *resultant.mutable_term1() = std::get<2>(t);
  *resultant.mutable_term2() = std::get<4>(t);
  *resultant.mutable_term3() = std::get<6>(t);

  PrintStep(tspan, "OP_TERNARY");
  *tspan = lcl;
  return resultant;
}

StatusOr<Expression> Parser::ConsumeExpression(TSpan *tspan) {
  TSpan lcl = *tspan;
  Expression resultant;

  absl::variant<Expression::OpUnary,   //
                Expression::OpBinary,  //
                Expression::OpTernary, //
                Expression,            //
                RangeLocation,         //
                PointLocation,         //
                Amount                 //
                >
      expression;

  ASSIGN_OR_RETURN_(
      expression,
      (AnyVariant<Expression::OpUnary,                               //
                  Expression::OpBinary,                              //
                  Expression::OpTernary,                             //
                  Expression,                                        //
                  RangeLocation,                                     //
                  PointLocation,                                     //
                  Amount                                             //
                  >(absl::bind_front(&Parser::ConsumeOpUnary, this), //
                    absl::bind_front(&Parser::ConsumeOpBinary,
                                     this), //
                    absl::bind_front(&Parser::ConsumeOpTernary,
                                     this),                                //
                    absl::bind_front(&Parser::ConsumeParentheses, this),   //
                    absl::bind_front(&Parser::ConsumeRangeLocation, this), //
                    absl::bind_front(&Parser::ConsumePointLocation, this), //
                    absl::bind_front(&Parser::ConsumeAmount, this)         //
                    )(&lcl)));

  std::visit(
      overload{
          [&resultant](Expression e) { resultant = e; },
          [&resultant](Expression::OpTernary o) {
            *resultant.mutable_op_ternary() = o;
          },
          [&resultant](Expression::OpBinary o) {
            *resultant.mutable_op_binary() = o;
          },
          [&resultant](Expression::OpUnary o) {
            *resultant.mutable_op_unary() = o;
          },
          [&resultant](RangeLocation rl) { *resultant.mutable_range() = rl; },
          [&resultant](PointLocation pl) { *resultant.mutable_lookup() = pl; },
          [&resultant](Amount value) { *resultant.mutable_value() = value; },
      },
      expression);

  PrintStep(tspan, "EXPRESSION");
  *tspan = lcl;
  return resultant;
}

} // namespace formula
} //  namespace latis
