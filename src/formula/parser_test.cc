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

using ::google::protobuf::TextFormat;
using ::testing::ContainerEq;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::Matches;
using ::testing::Not;
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

TEST(Parser, ConsumeStringOne) {
  const std::string input = "\"FOO bar _b^a^z_\"";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeString(&tspan), IsOkAndHolds("FOO bar _b^a^z_"));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeStringTwo) {
  const std::string input = "\"\"";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeString(&tspan), IsOkAndHolds(""));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeStringThree) {
  const std::string input = "";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeString(&tspan), Not(IsOk()));
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

TEST(Parser, Consume4DigitOne) {
  const std::string input = "2468";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(Consume4Digit(&tspan), IsOkAndHolds(2468));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, Consume4DigitTwo) {
  const std::string input = "9999AB";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(Consume4Digit(&tspan), IsOkAndHolds(9999));
}

TEST(Parser, Consume4DigitEmpty) {
  const std::string input = "";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(Consume4Digit(&tspan), Not(IsOk()));
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

TEST(Parser, ConsumeDateTime) {
  const std::string input = "2016-01-02T03:04:05.678+08:00";

  absl::Time expected_time;
  std::string msg;
  ASSERT_TRUE(absl::ParseTime(absl::RFC3339_full, input, &expected_time, &msg));

  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(DateTimeParser::ConsumeDateTime(&tspan),
              IsOkAndHolds(expected_time));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeDateTimeEmpty) {
  const std::string input = "";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(DateTimeParser::ConsumeDateTime(&tspan), Not(IsOk()));
}

TEST(Parser, ConsumeAmountString) {
  const std::string input = "\"foo\"";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(
      ConsumeAmount(&tspan),
      IsOkAndHolds(EqualsProto(ToProto<Amount>("str_amount: \"foo\""))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeAmountInt) {
  const std::string input = "240";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeAmount(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 240"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeAmountDouble) {
  const std::string input = "240.248";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(
      ConsumeAmount(&tspan),
      IsOkAndHolds(EqualsProto(ToProto<Amount>("double_amount: 240.248"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeAmountTimestamp) {
  const std::string input = "2016-01-02T03:04:05.678+08:00";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeAmount(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<Amount>(
                  "timestamp_amount: { seconds:1451675045 }"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeAmountMoney) {
  const std::string input = "$123.45";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeAmount(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<Amount>(
                  "money_amount: {dollars: 123 cents: 45 currency: USD}"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumePointLocationValidOne) {
  const std::string input = "A1";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(
      LocationParser::ConsumePointLocation(&tspan),
      IsOkAndHolds(EqualsProto(ToProto<PointLocation>("row: 0 col: 0"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumePointLocationValidTwo) {
  const std::string input = "AA100";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(
      LocationParser::ConsumePointLocation(&tspan),
      IsOkAndHolds(EqualsProto(ToProto<PointLocation>("row: 99 col: 26"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumePointLocationInvalidOne) {
  const std::string input = "AA";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumePointLocation(&tspan), Not(IsOk()));
}

TEST(Parser, ConsumePointLocationInvalidTwo) {
  const std::string input = "a1";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumePointLocation(&tspan), Not(IsOk()));
}

TEST(Parser, ConsumePointLocationInvalidThree) {
  const std::string input = "1A";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumePointLocation(&tspan), Not(IsOk()));
}

TEST(Parser, ConsumeRangeLocationValidOne) {
  const std::string input = "A1:B2";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumeRangeLocation(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<RangeLocation>(
                  "from_cell: {row: 0 col: 0} to_cell: {row: 1 col: 1}"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeRangeLocationValidTwo) {
  const std::string input = "A1:B";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumeRangeLocation(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<RangeLocation>(
                  "from_cell: {row: 0 col: 0} to_col: 1"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeRangeLocationValidThree) {
  const std::string input = "A1:2";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumeRangeLocation(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<RangeLocation>(
                  "from_cell: {row: 0 col: 0} to_row: 1"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeRangeLocationValidFour) {
  const std::string input = "A:B";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumeRangeLocation(&tspan),
              IsOkAndHolds(EqualsProto(
                  ToProto<RangeLocation>("from_col: 0 to_col: 1"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeRangeLocationValidFive) {
  const std::string input = "1:2";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumeRangeLocation(&tspan),
              IsOkAndHolds(EqualsProto(
                  ToProto<RangeLocation>("from_row: 0 to_row: 1"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeRangeLocationInvalidOne) {
  const std::string input = "1:A";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumeRangeLocation(&tspan), Not(IsOk()));
}

TEST(Parser, ConsumeRangeLocationInvalidTwo) {
  const std::string input = "1:";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumeRangeLocation(&tspan), Not(IsOk()));
}

TEST(Parser, ConsumeRangeLocationInvalidThree) {
  const std::string input = "A:";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(LocationParser::ConsumeRangeLocation(&tspan), Not(IsOk()));
}

TEST(Parser, ConsumeFnName) {
  const std::string input = "FOO";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(OperationParser::ConsumeFnName(&tspan), IsOkAndHolds("FOO"));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeFnNameTwo) {
  const std::string input = "FOO2";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(OperationParser::ConsumeFnName(&tspan), IsOkAndHolds("FOO2"));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeFnNameThree) {
  const std::string input = "FOO_2";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(OperationParser::ConsumeFnName(&tspan), IsOkAndHolds("FOO_2"));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeFnNameInvalidOne) {
  const std::string input = "2FOO";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(OperationParser::ConsumeFnName(&tspan), Not(IsOk()));
}

TEST(Parser, ConsumeFnNameInvalidEmpty) {
  const std::string input = "";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(OperationParser::ConsumeFnName(&tspan), Not(IsOk()));
}

TEST(Parser, ConsumeOpBinary) {
  const std::string input = "SUM(A1,A2)";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeExpression(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<Expression>(R"pb(
op_binary { 
  operation: "SUM" 
  term1: { lookup: { row: 0 col: 0 } } 
  term2: { lookup: { row: 1 col: 0 } }
})pb"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeOpBinaryNested) {
  const std::string input = "SUM(A1,SUM(A2,A3))";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};
  EXPECT_THAT(ConsumeExpression(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<Expression>(R"pb(
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
})pb"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeOpBinaryInfix) {
  const std::string input = "3+2";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};

  EXPECT_THAT(ConsumeExpression(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<Expression>(R"pb(
   op_binary {
    operation: "PLUS"
    term1: { value: { int_amount: 3 } }
    term2: { value: { int_amount: 2 } }
  })pb"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeParentheses) {
  const std::string input = "(3+2)";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};

  EXPECT_THAT(ConsumeExpression(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<Expression>(R"pb(
   op_binary {
    operation: "PLUS"
    term1: { value: { int_amount: 3 } }
    term2: { value: { int_amount: 2 } }
  })pb"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeOpBinaryNestedInfix) {
  const std::string input = "3+(2+1)";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};

  EXPECT_THAT(ConsumeExpression(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<Expression>(R"pb(
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
  })pb"))));
  EXPECT_THAT(tspan, IsEmpty());
}

TEST(Parser, ConsumeOpBinaryNestedInfixTwo) {
  const std::string input = "(3+2)+1";
  std::vector<Token> tokens;
  ASSERT_OK_AND_ASSIGN(tokens, Lex(input));
  TSpan tspan{tokens};

  ConsumeExpression(&tspan).ValueOrDie().PrintDebugString();
  EXPECT_THAT(ConsumeExpression(&tspan),
              IsOkAndHolds(EqualsProto(ToProto<Expression>(R"pb(
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
  })pb"))));
  EXPECT_THAT(tspan, IsEmpty());
}

// TODO(ambuc): many more expression tests for unary, binary, ternary
// TODO(ambuc: infix notation!

} // namespace
} // namespace formula
} // namespace latis
