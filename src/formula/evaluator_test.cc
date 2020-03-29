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

#include "src/formula/lexer.h"
#include "src/formula/parser.h"
#include "src/test_utils/test_utils.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace formula {
namespace {

using ::testing::Eq;
using ::testing::MockFunction;
using ::testing::Not;
using ::testing::ValuesIn;
using ::testing::WithParamInterface;

class TestClassBase
    : public ::testing::Test,
      public WithParamInterface<std::pair<std::string, std::string>> {
public:
  TestClassBase() : evaluator_(mock_lookup_fn_.AsStdFunction()) {
    EXPECT_CALL(mock_lookup_fn_, Call).Times(0);
  }

protected:
  MockFunction<absl::optional<Amount>(XY)> mock_lookup_fn_;
  Parser parser_;
  Evaluator evaluator_;
};

TEST_P(TestClassBase, LexAndParseAndEvaluate) {
  const std::string input = std::get<0>(GetParam());

  std::vector<Token> tokens = Lex(input).ValueOrDie();
  TSpan tspan{tokens};

  Expression expr = parser_.ConsumeExpression(&tspan).ValueOrDie();

  auto amt_or_status = evaluator_.CrunchExpression(expr);

  if (!amt_or_status.ok()) {
    std::cout << amt_or_status.status() << std::endl;
  }
  ASSERT_THAT(amt_or_status, IsOk());
  Amount amt = amt_or_status.ValueOrDie();

  EXPECT_THAT(amt, EqualsProto(ToProto<Amount>(std::get<1>(GetParam()))))
      << amt.DebugString();
}

INSTANTIATE_TEST_SUITE_P(
    All, TestClassBase,
    ValuesIn(std::vector<std::pair<std::string, std::string>>{
        {"1.234", "double_amount: 1.234"},
        {"\"FOO\"", "str_amount: \"FOO\""},
        // addition
        {"2 + 2", "int_amount: 4"},
        {"PLUS(2,3)", "int_amount: 5"},
        {"SUM(2.0,3)", "double_amount: 5.0"},
        {"ADD(1.5,1.5)", "double_amount: 3.0"},
        {"2.1 + 3", "double_amount: 5.1"},
        {"1.2 + 3.4", "double_amount: 4.6"},
        // subtraction
        {"5-2.5", "double_amount: 2.5"},
        {"SUB(3.0,2.5)", "double_amount: 0.5"},
        {"SUBTRACT(0,0)", "int_amount: 0"},
        {"MINUS(0.0,1.0)", "double_amount: -1.0"},
        // multiplication
        {"5*2.5", "double_amount: 12.5"},
        {"MULTIPLIED_BY(3.0,2.5)", "double_amount: 7.5"},
        {"TIMES(0,0)", "int_amount: 0"},
        {"PRODUCT(1.0,2.0)", "double_amount: 2.0"},
        // and
        {"True && True", "bool_amount: True"},
        {"True && False", "bool_amount: False"},
        {"False && True", "bool_amount: False"},
        {"False && False", "bool_amount: False"},
        // or
        {"True || True", "bool_amount: True"},
        {"True || False", "bool_amount: True"},
        {"False || True", "bool_amount: True"},
        {"False || False", "bool_amount: False"},
        // not
        {"NOT(True)", "bool_amount: False"},
        {"NOT(False)", "bool_amount: True"},
        // lthan
        {"1 < 2", "bool_amount: True"},
        {"1 < 1", "bool_amount: False"},
        // gthan
        {"2 > 1", "bool_amount: True"},
        {"1 > 1", "bool_amount: False"},
        // leq
        {" 1 <= 2 ", "bool_amount: True"},
        {" 2 <= 2 ", "bool_amount: True"},
        {" 3 <= 2 ", "bool_amount: False"},
    }));

} // namespace
} // namespace formula
} // namespace latis
