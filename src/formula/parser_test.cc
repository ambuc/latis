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
using ::testing::ValuesIn;
using ::testing::VariantWith;

template <typename T> //
class ConsumeTestSuiteBase : public ::testing::Test {
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
};

// INTEGER TEST SUITE

class IntegerTestSuite : public ConsumeTestSuiteBase<int>,
                         public ::testing::WithParamInterface<
                             std::pair<std::string, absl::optional<int>>> {
public:
  void Compare(const int &a, const int &b) const override { ASSERT_EQ(a, b); }
};
TEST_P(IntegerTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/ConsumeInt, std::get<0>(GetParam()),
                std::get<1>(GetParam()));
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
                        public ::testing::WithParamInterface<
                            std::pair<std::string, absl::optional<double>>> {
public:
  void Compare(const double &a, const double &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(DoubleTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/ConsumeDouble, std::get<0>(GetParam()),
                std::get<1>(GetParam()));
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<absl::variant<double, int>>>> {
public:
  void Compare(const absl::variant<double, int> &a,
               const absl::variant<double, int> &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(NumericTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/ConsumeNumeric, std::get<0>(GetParam()),
                std::get<1>(GetParam()));
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<Money::Currency>>> {
public:
  void Compare(const Money::Currency &a,
               const Money::Currency &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(CurrencyTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/MoneyParser::ConsumeCurrency,
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const Money &a, const Money &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(MoneyTestSuite, LexAndParse) {
  absl::optional<std::string> maybe_expectation = std::get<1>(GetParam());
  RunBodyOfTest(
      /*parser=*/MoneyParser::ConsumeMoney, std::get<0>(GetParam()),
      /*expectation_or_nullopt=*/maybe_expectation.has_value()
          ? absl::optional<Money>(ToProto<Money>(maybe_expectation.value()))
          : absl::nullopt);
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<absl::TimeZone>>> {
public:
  void Compare(const absl::TimeZone &a,
               const absl::TimeZone &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(TimeZoneTestSuite, LexAndParse) {
  RunBodyOfTest(
      /*parser=*/DateTimeParser::ConsumeTimeOffset, std::get<0>(GetParam()),
      std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllTimeZones, TimeZoneTestSuite,
    ValuesIn(
        std::vector<std::pair<std::string, absl::optional<absl::TimeZone>>>{
            {"+15:00", absl::FixedTimeZone(15 * 60 * 60)},
            {"-12:34", absl::FixedTimeZone(-1 * (12 * 60 * 60 + 34 * 60))},
            {"", absl::nullopt},
        }));

class TwoDigitTestSuite : public ConsumeTestSuiteBase<int>,
                          public ::testing::WithParamInterface<
                              std::pair<std::string, absl::optional<int>>> {
public:
  void Compare(const int &a, const int &b) const override { ASSERT_EQ(a, b); }
};
TEST_P(TwoDigitTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/Consume2Digit, std::get<0>(GetParam()),
                std::get<1>(GetParam()));
}
INSTANTIATE_TEST_SUITE_P(
    AllTwoDigitValues, TwoDigitTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<int>>>{
        {"24", 24},
        {"99", 99},
        {"999", absl::nullopt},
        {"", absl::nullopt},
    }));

class FourDigitTestSuite : public ConsumeTestSuiteBase<int>,
                           public ::testing::WithParamInterface<
                               std::pair<std::string, absl::optional<int>>> {
public:
  void Compare(const int &a, const int &b) const override { ASSERT_EQ(a, b); }
};
TEST_P(FourDigitTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/Consume4Digit, std::get<0>(GetParam()),
                std::get<1>(GetParam()));
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const std::string &a, const std::string &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(StringTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/ConsumeString, std::get<0>(GetParam()),
                std::get<1>(GetParam()));
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<absl::Time>>> {
public:
  void Compare(const absl::Time &a, const absl::Time &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(DateTimeTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/DateTimeParser::ConsumeDateTime,
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const Amount &a, const Amount &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(AmountTestSuite, LexAndParse) {
  absl::optional<std::string> maybe_expectation = std::get<1>(GetParam());
  RunBodyOfTest(
      /*parser=*/ConsumeAmount, std::get<0>(GetParam()),
      /*expectation_or_nullopt=*/maybe_expectation.has_value()
          ? absl::optional<Amount>(ToProto<Amount>(maybe_expectation.value()))
          : absl::nullopt);
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const PointLocation &a, const PointLocation &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(PointLocationTestSuite, LexAndParse) {
  absl::optional<std::string> maybe_expectation = std::get<1>(GetParam());
  RunBodyOfTest(
      /*parser=*/LocationParser::ConsumePointLocation, std::get<0>(GetParam()),
      /*expectation_or_nullopt=*/maybe_expectation.has_value()
          ? absl::optional<PointLocation>(
                ToProto<PointLocation>(maybe_expectation.value()))
          : absl::nullopt);
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const RangeLocation &a, const RangeLocation &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(RangeLocationTestSuite, LexAndParse) {
  absl::optional<std::string> maybe_expectation = std::get<1>(GetParam());
  RunBodyOfTest(
      /*parser=*/LocationParser::ConsumeRangeLocation, std::get<0>(GetParam()),
      /*expectation_or_nullopt=*/maybe_expectation.has_value()
          ? absl::optional<RangeLocation>(
                ToProto<RangeLocation>(maybe_expectation.value()))
          : absl::nullopt);
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const std::string &a, const std::string &b) const override {
    ASSERT_EQ(a, b);
  }
};
TEST_P(ConsumeFnNameTestSuite, LexAndParse) {
  absl::optional<std::string> maybe_expectation = std::get<1>(GetParam());
  RunBodyOfTest(
      /*parser=*/OperationParser::ConsumeFnName, std::get<0>(GetParam()),
      std::get<1>(GetParam()));
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
      public ::testing::WithParamInterface<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  void Compare(const Expression &a, const Expression &b) const override {
    ASSERT_THAT(a, EqualsProto(b));
  }
};
TEST_P(ExpressionTestSuite, LexAndParse) {
  absl::optional<std::string> maybe_expectation = std::get<1>(GetParam());
  RunBodyOfTest(
      /*parser=*/ConsumeExpression, std::get<0>(GetParam()),
      /*expectation_or_nullopt=*/maybe_expectation.has_value()
          ? absl::optional<Expression>(
                ToProto<Expression>(maybe_expectation.value()))
          : absl::nullopt);
}
INSTANTIATE_TEST_SUITE_P(
    AllExpressions, ExpressionTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"SUM(A1,A2)", R"pb(
op_binary {
  operation: "SUM"
  term1: { lookup: { row: 0 col: 0} }
  term2: { lookup: { row: 1 col: 0} }
}
)pb"},
        {"NEG(A1)", R"pb(
op_unary {
  operation: "NEG"
  term1: { lookup : { row : 0 col : 0 } }
}
)pb"},
        {"NEG(NEG(A1))", R"pb(
op_unary {
  operation: "NEG"
  term1: {
    op_unary {
      operation: "NEG"
      term1: { lookup: { row: 0 col: 0 } }
    }
  }
}
         )pb"},
        {"SUM(A1,SUM(A2,A3))", R"pb(
op_binary {
  operation: "SUM"
  term1: { lookup: { row: 0 col: 0 } }
  term2: {
    op_binary {
      operation: "SUM"
      term1: { lookup: { row: 1 col: 0 } }
      term2: { lookup: { row: 2 col: 0 } }
    }
  }
})pb"},

        {
            "3+2",
            R"pb(
op_binary {
  operation: "PLUS"
  term1: { value: { int_amount: 3 } }
  term2: { value: { int_amount: 2 } }
}
)pb",
        },

        {
            "(3+2)",
            R"pb(
           op_binary {
            operation: "PLUS"
            term1: { value: { int_amount: 3 } }
            term2: { value: { int_amount: 2 } }
          }
          )pb",
        },

        {
            "3+(2+1)",
            R"pb(
            op_binary {
             operation: "PLUS"
             term1: { value: { int_amount: 3 } }
             term2: {
              op_binary {
               operation: "PLUS"
               term1: { value: { int_amount: 2 } }
               term2: { value: { int_amount: 1 } }
              }
             }
           }
           )pb",
        },

        {
            "(3+2)+1",
            R"pb(
           op_binary {
            operation: "PLUS"
            term1: {
             op_binary {
              operation: "PLUS"
              term1: { value: { int_amount: 3 } }
              term2: { value: { int_amount: 2 } }
             }
            }
            term2: { value: { int_amount: 1 } }
          }
          )pb",
        },
    }));

// TODO(ambuc): many more expression tests for unary, binary,
// ternary
// TODO(ambuc: infix notation!

} // namespace
} // namespace formula
} // namespace latis
