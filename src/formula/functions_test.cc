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

#include "src/formula/functions.h"

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

class SumClassBase : public ::testing::Test,
                     public WithParamInterface<
                         std::tuple<std::string, std::string, std::string>> {};

TEST_P(SumClassBase, SumTrials) {
  const auto amt_or_status = ToProto<Amount>(std::get<0>(GetParam())) +
                             ToProto<Amount>(std::get<1>(GetParam()));

  const auto expected = ToProto<Amount>(std::get<2>(GetParam()));

  if (!amt_or_status.ok()) {
    std::cout << amt_or_status.status();
  }

  ASSERT_THAT(amt_or_status, IsOk());
  Amount amt = amt_or_status.ValueOrDie();

  EXPECT_THAT(amt, EqualsProto(expected)) << amt.DebugString();
}

INSTANTIATE_TEST_SUITE_P(
    All, SumClassBase,
    ValuesIn(std::vector<std::tuple<std::string, std::string, std::string>>{
        // 1 + 2 = 3
        {"int_amount: 1", "int_amount: 2", "int_amount: 3"},
        // 1.234 + 2.345 = 3.579
        {"double_amount: 1.234", "double_amount: 2.345",
         "double_amount: 3.579"},
        // "a" + "b" = "ab"
        {"str_amount: \"a\"", "str_amount: \"b\"", "str_amount: \"ab\""},
        // $1.23 + $2 = $3.23
        {"money_amount: { currency: USD dollars: 1 cents: 23 }",
         "money_amount: { currency: USD dollars: 2 }",
         "money_amount: { currency: USD dollars: 3 cents: 23 }"},
    }));

} // namespace
} // namespace formula
} // namespace latis
