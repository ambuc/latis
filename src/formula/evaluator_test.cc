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

class EvaluatorTestClass : public ::testing::Test {
public:
  LookupFn dummy_lookup_fn_ = [](XY) { return absl::nullopt; };
};

TEST_F(EvaluatorTestClass, Sample) {
  const std::string input = "=4.605";

  std::vector<Token> tokens = Lex(input).ValueOrDie();

  Expression expr;
  // EXPECT_THAT(Parse(tokens, &expr), IsOk());

  // Amount expected;
  // expected.set_double_amount(4.605);

  // Amount actual;

  // EXPECT_THAT(Evaluate(expr, dummy_lookup_fn_, &actual), IsOk());

  // EXPECT_THAT(actual, EqualsProto(expected));
}

} // namespace
} // namespace formula
} // namespace latis
