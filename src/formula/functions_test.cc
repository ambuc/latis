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

TEST(Sum, SumTrials) {
  const std::vector<Amount> inputs{
      ToProto<Amount>("int_amount: 1"),
      ToProto<Amount>("int_amount: 1"),
  };

  const absl::Span<const Amount> span = absl::MakeSpan(inputs);

  EXPECT_THAT(Sum(span),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 2"))));
}

} // namespace
} // namespace formula
} // namespace latis
