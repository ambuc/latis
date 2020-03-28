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

#include "src/formula/lexer.h"
#include "src/test_utils/test_utils.h"

#include "absl/functional/bind_front.h"
#include "absl/memory/memory.h"
#include "absl/time/time.h"
#include "google/protobuf/text_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace formula {
namespace {

using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::Test;
using ::testing::ValuesIn;
using ::testing::WithParamInterface;

namespace {

template <typename T> //
absl::optional<T> MaybeToProto(absl::optional<std::string> maybe_input) {
  if (maybe_input.has_value()) {
    return absl::optional<T>(ToProto<T>(maybe_input.value()));
  } else {
    return absl::nullopt;
  }
}

} // namespace

template <typename T> //
class ConsumeTestSuiteBase : public Test {
public:
  virtual void Compare(const T &a, const T &b) const = 0;

  // Ahh yes.... the garden of forking paths.
  void RunBodyOfTest(Prsr<T> parser, std::string input,
                     absl::optional<T> expectation_or_nullopt) {

    std::vector<Token> tokens;
    ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
    TSpan tspan{tokens};

    const auto object_or_status = parser(&tspan);

    if (expectation_or_nullopt.has_value()) {
      if (!object_or_status.ok()) {
        std::cout << object_or_status.status() << std::endl;
      }
      ASSERT_THAT(object_or_status, IsOk());
      Compare(object_or_status.ValueOrDie(), expectation_or_nullopt.value());
      ASSERT_THAT(tspan, IsEmpty());
    } else {
      ASSERT_FALSE(object_or_status.ok());
      if (!input.empty()) {
        ASSERT_THAT(tspan, Not(IsEmpty()));
      }
      ASSERT_THAT(tspan.size(), Eq(tokens.size()));
    }
  }

protected:
  Parser p_;
};

// INTEGER TEST SUITE

class IntegerTestSuite
    : public ConsumeTestSuiteBase<int>,
      public WithParamInterface<std::pair<std::string, absl::optional<int>>> {
public:
  void Compare(const int &a, const int &b) const override { ASSERT_EQ(a, b); }
};
TEST_P(IntegerTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeInt, &p_),
                std::get<0>(GetParam()), std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllIntegers, IntegerTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<int>>>{
        {"123", 123},
        {"0", 0},
        {"", absl::nullopt},
    }));

// DOUBLE TEST SUITE

class DoubleTestSuite : public ConsumeTestSuiteBase<double>,
                        public WithParamInterface<
                            std::pair<std::string, absl::optional<double>>> {
public:
  void Compare(const double &a, const double &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(DoubleTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeDouble, &p_),
                std::get<0>(GetParam()), std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllDoubles, DoubleTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<double>>>{
        {"4.605", 4.605},
        {".605", 0.605},
        {"42.", 42.0},
        {"", absl::nullopt},
    }));

// NUMERIC TEST SUITE

class NumericTestSuite
    : public ConsumeTestSuiteBase<absl::variant<double, int>>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<absl::variant<double, int>>>> {
public:
  void Compare(const absl::variant<double, int> &a,
               const absl::variant<double, int> &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(NumericTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeNumeric, &p_),
                std::get<0>(GetParam()), std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllNumeric, NumericTestSuite,
    ValuesIn(std::vector<std::pair<std::string,
                                   absl::optional<absl::variant<double, int>>>>{
        {"4.605", double(4.605)},
        {".605", 0.605},
        {"42", int(42)},
        {"", absl::nullopt},
    }));

// CURRENCY TEST SUITE

class CurrencyTestSuite
    : public ConsumeTestSuiteBase<Money::Currency>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<Money::Currency>>> {
public:
  void Compare(const Money::Currency &a,
               const Money::Currency &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(CurrencyTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeCurrency, &p_),
                std::get<0>(GetParam()), std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllCurrencies, CurrencyTestSuite,
    ValuesIn(
        std::vector<std::pair<std::string, absl::optional<Money::Currency>>>{
            {"$", Money::USD},
            {"USD", Money::USD},
            {"CAD", Money::CAD},
            {"", absl::nullopt},
        }));

class MoneyTestSuite
    : public ConsumeTestSuiteBase<Money>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const Money &a, const Money &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(MoneyTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeMoney, &p_),
                std::get<0>(GetParam()),
                MaybeToProto<Money>(std::get<1>(GetParam())));
}
INSTANTIATE_TEST_SUITE_P(
    AllMoney, MoneyTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"USD3", "dollars: 3 currency: USD"},
        {"CAD4.56", "dollars: 4 cents: 56 currency: CAD"},
        {"$123.456", "dollars: 123 cents: 45 currency: USD"},
        {"", absl::nullopt},
    }));

class TimeZoneTestSuite
    : public ConsumeTestSuiteBase<absl::TimeZone>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<absl::TimeZone>>> {
public:
  void Compare(const absl::TimeZone &a,
               const absl::TimeZone &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(TimeZoneTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeTimeOffset, &p_),
                std::get<0>(GetParam()), std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllTimeZones, TimeZoneTestSuite,
    ValuesIn(
        std::vector<std::pair<std::string, absl::optional<absl::TimeZone>>>{
            {"+15:00", absl::FixedTimeZone(15 * 60 * 60)},
            {"-12:34", absl::FixedTimeZone(-1 * (12 * 60 * 60 + 34 * 60))},
            {"", absl::nullopt},
        }));

class TwoDigitTestSuite
    : public ConsumeTestSuiteBase<int>,
      public WithParamInterface<std::pair<std::string, absl::optional<int>>> {
public:
  void Compare(const int &a, const int &b) const override { ASSERT_EQ(a, b); }
};
TEST_P(TwoDigitTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::Consume2Digit, &p_),
                std::get<0>(GetParam()), std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllTwoDigitValues, TwoDigitTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<int>>>{
        {"24", 24},
        {"99", 99},
        {"999", absl::nullopt},
        {"", absl::nullopt},
    }));

class FourDigitTestSuite
    : public ConsumeTestSuiteBase<int>,
      public WithParamInterface<std::pair<std::string, absl::optional<int>>> {
public:
  void Compare(const int &a, const int &b) const override { ASSERT_EQ(a, b); }
};
TEST_P(FourDigitTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::Consume4Digit, &p_),
                std::get<0>(GetParam()), std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllFourDigitValues, FourDigitTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<int>>>{
        {"2468", 2468},
        {"9999", 9999},
        {"99999", absl::nullopt},
        {"", absl::nullopt},
    }));

class StringTestSuite
    : public ConsumeTestSuiteBase<std::string>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const std::string &a, const std::string &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(StringTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeString, &p_),
                std::get<0>(GetParam()), std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllStringValues, StringTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"\"FOO bar _b&^a&^z_\"", "FOO bar _b&^a&^z_"},
        {"\"\"", ""},
        {"", absl::nullopt},
    }));

// CONSUME DATETIME TEST SUITE
class DateTimeTestSuite
    : public ConsumeTestSuiteBase<absl::Time>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<absl::Time>>> {
public:
  void Compare(const absl::Time &a, const absl::Time &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(DateTimeTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeDateTime, &p_),
                std::get<0>(GetParam()), std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllDateTimes, DateTimeTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<absl::Time>>>{
        {"2016-01-02T03:04:05.678+08:00",
         []() -> absl::Time {
           std::string input = "2016-01-02T03:04:05.678+08:00";
           absl::Time resultant;
           std::string msg;
           absl::ParseTime(absl::RFC3339_full, input, &resultant, &msg);
           return resultant;
         }()},
        {"", absl::nullopt},
    }));

// CONSUME AMOUNT TEST SUITE

class AmountTestSuite
    : public ConsumeTestSuiteBase<Amount>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const Amount &a, const Amount &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(AmountTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeAmount, &p_),
                std::get<0>(GetParam()),
                MaybeToProto<Amount>(std::get<1>(GetParam())));
}
INSTANTIATE_TEST_SUITE_P(
    AllAmounts, AmountTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"\"foo\"", "str_amount: \"foo\""},
        {"240", "int_amount: 240"},
        {"240.248", "double_amount: 240.248"},
        {"2016-01-02T03:04:05.678+08:00",
         "timestamp_amount: { seconds:1451675045 }"},
        {"$23", "money_amount: {dollars: 23 currency: USD}"},
        {"$123.45", "money_amount: {dollars: 123 cents: 45 currency: USD}"},
    }));

// POINT LOCATION TEST SUITE

class PointLocationTestSuite
    : public ConsumeTestSuiteBase<PointLocation>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const PointLocation &a, const PointLocation &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(PointLocationTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumePointLocation, &p_),
                std::get<0>(GetParam()),
                MaybeToProto<PointLocation>(std::get<1>(GetParam())));
}
INSTANTIATE_TEST_SUITE_P(
    AllPointLocations, PointLocationTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"A1", "row: 0 col: 0"},
        {"AA100", "row: 99 col: 26"},
        {"AA", absl::nullopt},
        {"a1", absl::nullopt},
        {"1A", absl::nullopt},
    }));

// RANGE LOCATION TEST SUITE

class RangeLocationTestSuite
    : public ConsumeTestSuiteBase<RangeLocation>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const RangeLocation &a, const RangeLocation &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(RangeLocationTestSuite, LexAndParse) {
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeRangeLocation, &p_),
                std::get<0>(GetParam()),
                MaybeToProto<RangeLocation>(std::get<1>(GetParam())));
}
INSTANTIATE_TEST_SUITE_P(
    AllRangeLocations, RangeLocationTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"A1:B2", "from_cell: { row: 0 col: 0 } to_cell: { "
                  "row: 1 col: 1 }"},
        {"A1:B", "from_cell: { row: 0 col: 0 } to_col: 1"},
        {"A1:2", "from_cell: { row: 0 col: 0 } to_row: 1"},
        {"A:B", "from_col: 0 to_col: 1"},
        {"1:2", "from_row: 0 to_row: 1"},
        {"1:A", absl::nullopt},
        {"1:", absl::nullopt},
        {"A:", absl::nullopt},
    }));

// CONSUME FN NAME TEST SUITE

class ConsumeFnNameTestSuite
    : public ConsumeTestSuiteBase<std::string>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const std::string &a, const std::string &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(ConsumeFnNameTestSuite, LexAndParse) {
  absl::optional<std::string> maybe_expectation = std::get<1>(GetParam());
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeFnName, &p_),
                std::get<0>(GetParam()), std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllFnNames, ConsumeFnNameTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"FOO", "FOO"},
        {"FOO2", "FOO2"},
        {"FOO_2", "FOO_2"},
        {"2FOO", absl::nullopt},
        {"?", absl::nullopt},
    }));

// EXPRESSION TEST SUITE

class ExpressionTestSuite
    : public ConsumeTestSuiteBase<Expression>,
      public WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const Expression &a, const Expression &b) const override {
    ASSERT_THAT(a, EqualsProto(b))
        << "Actual: " << a.DebugString() << ", Expected: " << b.DebugString();
  }
};

TEST_P(ExpressionTestSuite, LexAndParse) {
  p_.EnableVerboseLogging(); // TODO remove this.
  RunBodyOfTest(absl::bind_front(&Parser::ConsumeExpression, &p_),
                std::get<0>(GetParam()),
                MaybeToProto<Expression>(std::get<1>(GetParam())));
}

INSTANTIATE_TEST_SUITE_P(
    Amounts, ExpressionTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"2", R"pb(value { int_amount: 2 })pb"},
        {"3.0", R"pb(value { double_amount: 3.0 })pb"},
    }));

INSTANTIATE_TEST_SUITE_P(
    Parentheses, ExpressionTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        // Parentheses around a bunch of single values.
        {"(2)", R"pb(value { int_amount: 2 })pb"},
        {"(3.0)", R"pb(value { double_amount: 3.0 })pb"},
        {R"pb(("s"))pb", R"pb(value { str_amount: "s" })pb"},
    }));

INSTANTIATE_TEST_SUITE_P(
    UnaryPrefix, ExpressionTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        // Point and Range locations.
        {R"pb(WALDO(A1))pb",
         R"pb(operation { fn_name: "WALDO" terms: { lookup: { row : 0 col : 0 } } })pb"},
        {R"pb(GARPLY(A1:B2))pb",
         R"pb(operation { fn_name: "GARPLY" terms: { range: { from_cell: { row: 0 col: 0 } to_cell: { row: 1  col: 1} } } })pb"},

        // Basic amounts.
        {R"(FOO("foo"))",
         R"pb(operation { fn_name: "FOO" terms: { value: { str_amount: "foo" } } })pb"},
        {R"(BAR(240))",
         R"pb(operation { fn_name: "BAR" terms: { value: { int_amount: 240 } } })pb"},
        {R"(BAZ(240.248))",
         R"pb(operation { fn_name: "BAZ" terms: { value: { double_amount: 240.248 } } })pb"},
        {R"(QUX(2016-01-02T03:04:05.678+08:00))",
         R"pb(operation { fn_name: "QUX" terms: { value: { timestamp_amount: { seconds: 1451675045 } } } })pb"},
        {R"(CORGE($23))",
         R"pb(operation { fn_name: "CORGE" terms: { value: { money_amount: { dollars: 23 currency: USD } } } })pb"},
        {R"(GRAULT($123.45))",
         R"pb(operation { fn_name: "GRAULT" terms: { value: { money_amount: { dollars: 123 cents: 45 currency: USD } } } })pb"},

        // Nested unary prefix operations.
        {R"(FOO1(FOO2(3)))",
         R"pb(operation { fn_name: "FOO1" terms: { operation: { fn_name: "FOO2" terms: { value: { int_amount: 3 } } } } })pb"},
        {R"(FOO1(FOO2("3")))",
         R"pb(operation { fn_name: "FOO1" terms: { operation: { fn_name: "FOO2" terms: { value: { str_amount: "3" } } } } })pb"},

        // Ignoring the inner parentheses of a string.
        {R"pb(FOO1("FOO2(3)"))pb",
         R"pb(operation { fn_name: "FOO1" terms: { value: { str_amount: "FOO2(3)" } } })pb"},
        // Ignoring the inner parentheses of a string, even if empty.
        {R"pb(FOO1(""))pb",
         R"pb(operation { fn_name: "FOO1" terms: { value: { str_amount: "" } } })pb"},
    }));

const std::string PARENTHESES_AROUND_UNARY_PREFIX_EXPECTATION = std::string(
    R"pb(operation { fn_name: "FOO1" terms: { value: { int_amount: 2 } } })pb");
INSTANTIATE_TEST_SUITE_P(
    ParenthesesAroundUnaryPrefix, ExpressionTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {R"pb(FOO1(2))pb", PARENTHESES_AROUND_UNARY_PREFIX_EXPECTATION},
        {R"pb((FOO1(2)))pb", PARENTHESES_AROUND_UNARY_PREFIX_EXPECTATION},
        {R"pb(FOO1((2)))pb", PARENTHESES_AROUND_UNARY_PREFIX_EXPECTATION},
        {R"pb((FOO1((2))))pb", PARENTHESES_AROUND_UNARY_PREFIX_EXPECTATION},
        {R"pb((FOO1(((2)))))pb", PARENTHESES_AROUND_UNARY_PREFIX_EXPECTATION},
    }));

INSTANTIATE_TEST_SUITE_P(
    BinaryPrefix, ExpressionTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"FOO(1,2.0)",
         R"pb(operation { fn_name: "FOO" terms: { value: { int_amount: 1 } } terms: { value: { double_amount: 2.0 } } })pb"},

        {R"pb(FOO("BAR", $3.45))pb",
         R"pb(operation { fn_name: "FOO" terms: { value: { str_amount: "BAR" } } terms: { value: { money_amount: { dollars: 3 cents: 45 currency: USD } } } })pb"},

        // Ignoring the inner parentheses of a string.
        {R"pb(FOO("B(AR", "BA)Z"))pb",
         R"pb(operation { fn_name: "FOO" terms: { value: { str_amount: "B(AR" } } terms: { value: { str_amount: "BA)Z" } } })pb"},

        // Nested in the former.
        {"BAR(BAZ(1,2),3)",
         R"pb(operation { fn_name: "BAR" terms: { operation { fn_name: "BAZ" terms: { value : { int_amount: 1 } } terms: { value : { int_amount: 2 } } } } terms: { value : { int_amount: 3 } } })pb"},

        // Nested in the latter.
        {"BAR(1,BAZ(2,3))",
         R"pb(operation { fn_name: "BAR" terms: { value: { int_amount: 1 } } terms: { operation { fn_name: "BAZ" terms: { value: { int_amount: 2 } } terms: { value: { int_amount: 3 } } } } })pb"},

        // Nested in both.
        {"BAR(FOO(1,2),BAZ(3,4))",
         R"pb(operation { fn_name: "BAR" terms: { operation { fn_name: "FOO" terms: { value: { int_amount: 1 } } terms: { value: { int_amount: 2 } } } } terms: { operation { fn_name: "BAZ" terms: { value: { int_amount: 3 } } terms: { value: { int_amount: 4 } } } } })pb"},
    }));

const std::string PARENTHESES_AROUND_BINARY_PREFIX_EXPECTATION = std::string(
    R"pb(operation { fn_name: "FOO" terms: { value: { int_amount: 1 } }
     terms: { value: { double_amount: 2.0 } } })pb");
INSTANTIATE_TEST_SUITE_P(
    ParenthesesAroundBinaryPrefix, ExpressionTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"FOO(1,2.0)", PARENTHESES_AROUND_BINARY_PREFIX_EXPECTATION},
        {"FOO((1),2.0)", PARENTHESES_AROUND_BINARY_PREFIX_EXPECTATION},
        {"FOO(1,(2.0))", PARENTHESES_AROUND_BINARY_PREFIX_EXPECTATION},
        {"FOO((1),(2.0))", PARENTHESES_AROUND_BINARY_PREFIX_EXPECTATION},
        {"(FOO((1),(2.0)))", PARENTHESES_AROUND_BINARY_PREFIX_EXPECTATION},
    }));

INSTANTIATE_TEST_SUITE_P(
    BinaryInfix, ExpressionTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        // Infix.
        {"3+2",
         R"pb( operation { fn_name: "PLUS" terms: { value: { int_amount: 3 } } terms: { value: { int_amount: 2 } } })pb"},

        {"3+2.0",
         R"pb( operation { fn_name: "PLUS" terms: { value: { int_amount: 3 } } terms: { value: { double_amount: 2.0 } } })pb"},

        // Infix with strings? And a different operator?
        {R"pb("3" / "2")pb",
         R"pb( operation { fn_name: "DIVIDED_BY" terms: { value: { str_amount: "3" } } terms: { value: { str_amount: "2" } } })pb"},

        {R"pb(0.0 - 2.0)pb",
         R"pb( operation { fn_name: "MINUS" terms: { value: { double_amount: 0.0 } } terms: { value: { double_amount: 2.0 } } })pb"},

        // Infix with parens?
        // TODO FIXME broken
        {R"pb(((0.0)-(2.0)))pb",
         R"pb( operation { fn_name: "MINUS" terms: { value: { double_amount: 0.0 } } terms: { value: { double_amount: 2.0 } } })pb"},

        // Infix with parens?
        // TODO FIXME broken
        {R"pb((0.0)-(2.0))pb",
         R"pb( operation { fn_name: "MINUS" terms: { value: { double_amount: 0.0 } } terms: { value: { double_amount: 2.0 } } })pb"},

        {"(3+2)",
         R"pb( operation { fn_name: "PLUS" terms: { value: { int_amount: 3 } } terms: { value: { int_amount: 2 } } })pb"},

        {"3+(2+1)",
         R"pb( operation { fn_name: "PLUS" terms: { value: { int_amount: 3 } } terms: { operation { fn_name: "PLUS" terms: { value: { int_amount: 2 } } terms: { value: { int_amount: 1 } } } } })pb"},

        // TODO FIXME broken
        {"(3+2)+1)",
         R"pb( operation { fn_name: "PLUS" terms: { operation { fn_name: "PLUS" terms: { value: { int_amount: 3 } } terms: { value: { int_amount: 2 } } } } terms: { value: { int_amount: 1 } } })pb"},
    }));

// TODO(ambuc): many more expression tests for unary, binary, ternary
// TODO(ambuc: infix notation!

} // namespace
} // namespace formula
} // namespace latis
