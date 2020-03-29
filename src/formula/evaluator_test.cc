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

  ASSERT_THAT(amt_or_status, IsOk());
  Amount amt = amt_or_status.ValueOrDie();

  EXPECT_THAT(amt, EqualsProto(ToProto<Amount>(std::get<1>(GetParam()))));
}

INSTANTIATE_TEST_SUITE_P(
    All, TestClassBase,
    ValuesIn(std::vector<std::pair<std::string, std::string>>{
        {"1.234", "double_amount: 1.234"},
        {"\"FOO\"", "str_amount: \"FOO\""},
        {"2 + 2", "int_amount: 4"},
    }));

} // namespace
} // namespace formula
} // namespace latis
