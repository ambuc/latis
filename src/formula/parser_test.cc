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

TEST(Parser, ConsumeInteger) {
  const std::string input = "123";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeInt(&tspan), IsOkAndHolds(123));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeIntegerEmpty) {
  const std::string input = "";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeInt(&tspan), Not(IsOk()));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeDoubleOne) {
  const std::string input = "4.605";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeDouble(&tspan), IsOkAndHolds(4.605));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeDoubleTwo) {
  const std::string input = ".605";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeDouble(&tspan), IsOkAndHolds(0.605));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeDoubleThree) {
  const std::string input = "42.";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeDouble(&tspan), IsOkAndHolds(42.0));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeDoubleEmpty) {
  const std::string input = "";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeDouble(&tspan), Not(IsOk()));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeNumericOne) {
  const std::string input = "42";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeNumeric(&tspan), IsOkAndHolds(VariantWith<int>(42)));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeNumericTwo) {
  const std::string input = "42.234";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeNumeric(&tspan),
              IsOkAndHolds(VariantWith<double>(42.234)));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeNumericEmpty) {
  const std::string input = "";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeNumeric(&tspan), Not(IsOk()));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeCurrencyUSD) {
  const std::string input = "USD";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(MoneyParser::ConsumeCurrency(&tspan), IsOkAndHolds(Money::USD));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeCurrencyCAD) {
  const std::string input = "CAD";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(MoneyParser::ConsumeCurrency(&tspan), IsOkAndHolds(Money::CAD));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeCurrencyDollarSign) {
  const std::string input = "$";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(MoneyParser::ConsumeCurrency(&tspan), IsOkAndHolds(Money::USD));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeCurrencyEmpty) {
  const std::string input = "";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(MoneyParser::ConsumeCurrency(&tspan), Not(IsOk()));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeMoneyOne) {
  const std::string input = "USD3";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(
      MoneyParser::ConsumeMoney(&tspan),
      IsOkAndHolds(EqualsProto(ToProto<Money>("dollars: 3 currency: USD"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeMoneyTwo) {
  const std::string input = "CAD4.56";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(MoneyParser::ConsumeMoney(&tspan),
              IsOkAndHolds(EqualsProto(
                  ToProto<Money>("dollars: 4 cents: 56 currency: CAD"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeMoneyThree) {
  const std::string input = "$123.456";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(MoneyParser::ConsumeMoney(&tspan),
              IsOkAndHolds(EqualsProto(
                  ToProto<Money>("dollars: 123 cents: 45 currency: USD"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, Consume2DigitOne) {
  const std::string input = "24";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(Consume2Digit(&tspan), IsOkAndHolds(24));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, Consume2DigitTwo) {
  const std::string input = "99AB";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(Consume2Digit(&tspan), IsOkAndHolds(99));
}

TEST(Parser, Consume2DigitEmpty) {
  const std::string input = "";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(Consume2Digit(&tspan), Not(IsOk()));
}

TEST(Parser, ConsumeTimeOffset) {
  const std::string input = "+15:00";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(DateTimeParser::ConsumeTimeOffset(&tspan),
              IsOkAndHolds(absl::FixedTimeZone(15 * 60 * 60)));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeTimeOffsetTwo) {
  const std::string input = "-12:34";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(DateTimeParser::ConsumeTimeOffset(&tspan),
              IsOkAndHolds(absl::FixedTimeZone(-1 * (12 * 60 * 60 + 34 * 60))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeTimeOffsetEmpty) {
  const std::string input = "";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(DateTimeParser::ConsumeTimeOffset(&tspan), Not(IsOk()));
}

template <typename T> //
class ConsumeTestSuiteBase
    : public testing::TestWithParam<
          std::pair<std::string, absl::optional<std::string>>> {
public:
  virtual void Compare(const T &a, const T &b) const = 0;

  // Ahh yes.... the garden of forking paths.
  void RunBodyOfTest(Prsr<T> parser, std::function<T(std::string)> mangler) {
    std::string input = std::get<0>(GetParam());
    absl::optional<std::string> expectation_or_nullopt =
        std::get<1>(GetParam());

    std::vector<Token> tokens;
    ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
    TSpan tspan{tokens};

    const auto thing_or_status = parser(&tspan);

    if (expectation_or_nullopt.has_value()) {
      T expectation = mangler(expectation_or_nullopt.value());

      if (!thing_or_status.ok()) {
        std::cout << thing_or_status.status();
      }
      EXPECT_TRUE(thing_or_status.ok());

      const T actual = thing_or_status.ValueOrDie();

      Compare(actual, expectation);

      EXPECT_THAT(tspan, IsEmpty());
    } else {
      EXPECT_FALSE(thing_or_status.ok());

      if (!input.empty()) {
        EXPECT_THAT(tspan, Not(IsEmpty()));
      }

      EXPECT_THAT(tspan.size(), Eq(tokens.size()));
    }
  }
};

class TwoDigitTestSuite : public ConsumeTestSuiteBase<int> {
public:
  void Compare(const int &a, const int &b) const override { EXPECT_EQ(a, b); }
};
TEST_P(TwoDigitTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/Consume2Digit, /*mangler=*/[](std::string s) -> int {
    int i;
    (void)absl::SimpleAtoi(s, &i);
    return i;
  });
}
INSTANTIATE_TEST_SUITE_P(
    AllTwoDigitValues, TwoDigitTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"24", "24"},
        {"99", "99"},
        {"999", absl::nullopt},
        {"", absl::nullopt},
    }));

class FourDigitTestSuite : public ConsumeTestSuiteBase<int> {
public:
  void Compare(const int &a, const int &b) const override { EXPECT_EQ(a, b); }
};
TEST_P(FourDigitTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/Consume4Digit, /*mangler=*/[](std::string s) -> int {
    int i;
    (void)absl::SimpleAtoi(s, &i);
    return i;
  });
}
INSTANTIATE_TEST_SUITE_P(
    AllFourDigitValues, FourDigitTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"2468", "2468"},
        {"9999", "9999"},
        {"99999", absl::nullopt},
        {"", absl::nullopt},
    }));

class StringTestSuite : public ConsumeTestSuiteBase<std::string> {
public:
  void Compare(const std::string &a, const std::string &b) const override {
    EXPECT_EQ(a, b);
  }
};
TEST_P(StringTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/ConsumeString,
                /*mangler=*/[](std::string i) { return i; });
}
INSTANTIATE_TEST_SUITE_P(
    AllStringValues, StringTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"\"FOO bar _b&^a&^z_\"", "FOO bar _b&^a&^z_"},
        {"\"\"", ""},
        {"", absl::nullopt},
    }));

// CONSUME DATETIME TEST SUITE
class DateTimeTestSuite : public ConsumeTestSuiteBase<absl::Time> {
public:
  void Compare(const absl::Time &a, const absl::Time &b) const override {
    EXPECT_EQ(a, b);
  }
};
TEST_P(DateTimeTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/DateTimeParser::ConsumeDateTime,
                /*mangler=*/[](std::string s) -> absl::Time {
                  absl::Time resultant;
                  std::string msg;
                  absl::ParseTime(absl::RFC3339_full, s, &resultant, &msg);
                  return resultant;
                });
}
INSTANTIATE_TEST_SUITE_P(
    AllDateTimes, DateTimeTestSuite,
    ValuesIn(std::vector<std::pair<std::string, absl::optional<std::string>>>{
        {"2016-01-02T03:04:05.678+08:00", "2016-01-02T03:04:05.678+08:00"},
        {"", absl::nullopt},
    }));

// CONSUME AMOUNT TEST SUITE

class AmountTestSuite : public ConsumeTestSuiteBase<Amount> {
public:
  void Compare(const Amount &a, const Amount &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(AmountTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/ConsumeAmount, /*mangler=*/ToProto<Amount>);
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

class PointLocationTestSuite : public ConsumeTestSuiteBase<PointLocation> {
public:
  void Compare(const PointLocation &a, const PointLocation &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(PointLocationTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/LocationParser::ConsumePointLocation,
                /*mangler=*/ToProto<PointLocation>);
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

class RangeLocationTestSuite : public ConsumeTestSuiteBase<RangeLocation> {
public:
  void Compare(const RangeLocation &a, const RangeLocation &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(RangeLocationTestSuite, LexAndParse) {
  RunBodyOfTest(
      /*parser=*/LocationParser::ConsumeRangeLocation,
      /*mangler=*/ToProto<RangeLocation>);
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

class ConsumeFnNameTestSuite : public ConsumeTestSuiteBase<std::string> {
public:
  void Compare(const std::string &a, const std::string &b) const override {
    EXPECT_EQ(a, b);
  }
};
TEST_P(ConsumeFnNameTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/OperationParser::ConsumeFnName,
                [](auto i) { return i; });
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

class ExpressionTestSuite : public ConsumeTestSuiteBase<Expression> {
public:
  void Compare(const Expression &a, const Expression &b) const override {
    EXPECT_THAT(a, EqualsProto(b));
  }
};
TEST_P(ExpressionTestSuite, LexAndParse) {
  RunBodyOfTest(/*parser=*/ConsumeExpression,
                /*mangler=*/ToProto<Expression>);
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
        {"3+2", R"pb(
op_binary {
  operation: "PLUS"
  term1: { value: { int_amount: 3 } }
  term2: { value: { int_amount: 2 } }
}
)pb"},

        //        {"(3+2)", R"pb(
        //   op_binary {
        //    operation: "PLUS"
        //    term1: { value: { int_amount: 3 } }
        //    term2: { value: { int_amount: 2 } }
        //  }
        //  )pb"},

        // {"3+(2+1)", R"pb(
        //     op_binary {
        //      operation: "PLUS"
        //      term1: { value: { int_amount: 3 } }
        //      term2: {
        //       op_binary {
        //        operation: "PLUS"
        //        term1: { value: { int_amount: 2 } }
        //        term2: { value: { int_amount: 1 } }
        //       }
        //      }
        //    }
        //    )pb"},

        // {"(3+2)+1", R"pb(
        //    op_binary {
        //     operation: "PLUS"
        //     term1: {
        //      op_binary {
        //       operation: "PLUS"
        //       term1: { value: { int_amount: 3 } }
        //       term2: { value: { int_amount: 2 } }
        //      }
        //     }
        //     term2: { value: { int_amount: 1 } }
        //   }
        //   )pb"},
    }));

// TODO(ambuc): many more expression tests for unary, binary,
// ternary
// TODO(ambuc: infix notation!

} // namespace
} // namespace formula
} // namespace latis
