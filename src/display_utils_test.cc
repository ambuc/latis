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

#include "src/display_utils.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace {

using ::testing::Eq;

TEST(PrintCell, StringAmount) {
  Cell c;
  c.mutable_amount()->set_str_amount("foo");
  EXPECT_THAT(PrintCell(c), Eq("foo"));
}

TEST(PrintCell, IntAmount) {
  Cell c;
  c.mutable_amount()->set_int_amount(1234);
  EXPECT_THAT(PrintCell(c), Eq("1234"));
}

TEST(PrintCell, DoubleAmount) {
  Cell c;
  c.mutable_amount()->set_double_amount(12.34567890);
  EXPECT_THAT(PrintCell(c), Eq("12.35"));
}

TEST(PrintCell, TimestampAmount) {
  Cell c;
  c.mutable_amount()->mutable_timestamp_amount()->set_seconds(0);
  EXPECT_THAT(PrintCell(c), Eq("1970-01-01T00:00:00+00:00"));
}

TEST(PrintCell, MoneyAmount) {
  Cell c;

  auto *money = c.mutable_amount()->mutable_money_amount();
  money->set_currency(Money::USD);

  money->set_dollars(10);
  money->set_cents(10);
  EXPECT_THAT(PrintCell(c), Eq("$10.10"));

  money->set_dollars(0);
  EXPECT_THAT(PrintCell(c), Eq("$0.10"));

  money->set_cents(0);
  EXPECT_THAT(PrintCell(c), Eq("$0.00"));
}

} // namespace
} // namespace latis
