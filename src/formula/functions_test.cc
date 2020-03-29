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

#include "absl/types/optional.h"
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

using Params =
    std::tuple<std::string, std::string, absl::optional<std::string>>;

class TestSuite : public ::testing::Test, public WithParamInterface<Params> {
public:
  virtual StatusOr<Amount> Combine(const Amount &lhs, const Amount &rhs) = 0;

  void RunTest() {
    Amount lhs = ToProto<Amount>(std::get<0>(GetParam()));
    Amount rhs = ToProto<Amount>(std::get<1>(GetParam()));

    const auto amt_or_status = Combine(lhs, rhs);

    if (const auto maybe_expected_str = std::get<2>(GetParam());
        !maybe_expected_str.has_value()) {
      EXPECT_THAT(amt_or_status, Not(IsOk()));
      if (amt_or_status.ok()) {
        std::cout << amt_or_status.ValueOrDie().DebugString() << std::endl;
      }
    } else {
      const auto expected = ToProto<Amount>(maybe_expected_str.value());

      if (!amt_or_status.ok()) {
        std::cout << amt_or_status.status();
      }

      ASSERT_THAT(amt_or_status, IsOk());
      Amount amt = amt_or_status.ValueOrDie();

      EXPECT_THAT(amt, EqualsProto(expected)) << amt.DebugString();
    }
  }
};

class AdditionTestSuite : public TestSuite {
  StatusOr<Amount> Combine(const Amount &lhs, const Amount &rhs) {
    return lhs + rhs;
  }
};

TEST_P(AdditionTestSuite, RunTests) { RunTest(); }

INSTANTIATE_TEST_SUITE_P(
    AllTests, AdditionTestSuite,
    ValuesIn(std::vector<Params>{
        {"int_amount: 1", "int_amount: 2", "int_amount: 3"},
        {"int_amount: 1", "double_amount: 2.0", "double_amount: 3.0"},
        {"double_amount: 1.0", "int_amount: 2", "double_amount: 3.0"},
        {"double_amount: 2.1", "int_amount: 3", "double_amount: 5.1"},
        {"double_amount: 1.234", "double_amount: 2.345",
         "double_amount: 3.579"},
        // "a" + "b" = "ab"
        {"str_amount: \"a\"", "str_amount: \"b\"", "str_amount: \"ab\""},
        // $1.23 + $2 = $3.23
        {"money_amount: { currency: USD dollars: 1 cents: 23 }",
         "money_amount: { currency: USD dollars: 2 }",
         "money_amount: { currency: USD dollars: 3 cents: 23 }"},

        // INVALID
        {"int_amount : 1", "str_amount: \"a\"", absl::nullopt},
        {"int_amount : 1", "money_amount: {} ", absl::nullopt},
        {"int_amount : 1", "timestamp_amount: {} ", absl::nullopt},
        {"str_amount: \"a\"", "timestamp_amount: {} ", absl::nullopt},
        {"str_amount: \"a\"", "money_amount: {} ", absl::nullopt},
        {"timestamp_amount: {}", "money_amount: {} ", absl::nullopt},
    }));

class SubtractionTestSuite : public TestSuite {
  StatusOr<Amount> Combine(const Amount &lhs, const Amount &rhs) {
    return lhs - rhs;
  }
};
TEST_P(SubtractionTestSuite, RunTests) { RunTest(); }
INSTANTIATE_TEST_SUITE_P(
    AllTests, SubtractionTestSuite,
    ValuesIn(std::vector<Params>{
        {"int_amount: 3", "int_amount: 1", "int_amount: 2"},
        {"int_amount: 1", "int_amount: 1", "int_amount: 0"},
        {"int_amount: 0", "int_amount: 0", "int_amount: 0"},
        {"double_amount: 3.0", "int_amount: 1", "double_amount: 2.0"},

        // $2.23 + $1 = $1.23
        {"money_amount: { currency: USD dollars: 2 cents: 23 }",
         "money_amount: { currency: USD dollars: 1 }",
         "money_amount: { currency: USD dollars: 1 cents: 23 }"},

        // string subtraction is bogus
        {"str_amount: \"a\"", "str_amount: \"b\"", absl::nullopt},

        // Negative
        {"int_amount: 1", "int_amount: 2", "int_amount: -1"},
        {"double_amount: 1.0", "double_amount: 2.0", "double_amount: -1.0"},

        // INVALID
        {"int_amount : 1", "str_amount: \"a\"", absl::nullopt},
        {"int_amount : 1", "money_amount: {} ", absl::nullopt},
        {"int_amount : 1", "timestamp_amount: {} ", absl::nullopt},
        {"str_amount: \"a\"", "timestamp_amount: {} ", absl::nullopt},
        {"str_amount: \"a\"", "money_amount: {} ", absl::nullopt},
        {"timestamp_amount: {}", "money_amount: {} ", absl::nullopt},
    }));

class BooleanAndTestSuite : public TestSuite {
  StatusOr<Amount> Combine(const Amount &lhs, const Amount &rhs) {
    return lhs && rhs;
  }
};
TEST_P(BooleanAndTestSuite, RunTests) { RunTest(); }
INSTANTIATE_TEST_SUITE_P(
    AllTests, BooleanAndTestSuite,
    ValuesIn(std::vector<Params>{
        {"bool_amount: true", "bool_amount: true", "bool_amount: true"},
        {"bool_amount: true", "bool_amount: false", "bool_amount: false"},
        {"bool_amount: false", "bool_amount: true", "bool_amount: false"},
        {"bool_amount: false", "bool_amount: false", "bool_amount: false"},
    }));

class BooleanOrTestSuite : public TestSuite {
  StatusOr<Amount> Combine(const Amount &lhs, const Amount &rhs) {
    return lhs || rhs;
  }
};
TEST_P(BooleanOrTestSuite, RunTests) { RunTest(); }
INSTANTIATE_TEST_SUITE_P(
    AllTests, BooleanOrTestSuite,
    ValuesIn(std::vector<Params>{
        {"bool_amount: true", "bool_amount: true", "bool_amount: true"},
        {"bool_amount: true", "bool_amount: false", "bool_amount: true"},
        {"bool_amount: false", "bool_amount: true", "bool_amount: true"},
        {"bool_amount: false", "bool_amount: false", "bool_amount: false"},
    }));

class BooleanNotTestSuite : public TestSuite {
  StatusOr<Amount> Combine(const Amount &lhs, const Amount &) { return !lhs; }
};
TEST_P(BooleanNotTestSuite, RunTests) { RunTest(); }
// HACK. TODO(ambuc): unary test suites too.
INSTANTIATE_TEST_SUITE_P(AllTests, BooleanNotTestSuite,
                         ValuesIn(std::vector<Params>{
                             {"bool_amount: true", "", "bool_amount: false"},
                             {"bool_amount: false", "", "bool_amount: true"},
                         }));

} // namespace
} // namespace formula
} // namespace latis
