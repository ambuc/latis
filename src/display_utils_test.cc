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

#include <iostream>

#include "src/test_utils/test_utils.h"

#include "absl/container/flat_hash_map.h"
#include "src/xy.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace {

using ::testing::Eq;

TEST(PrintCell, StringAmount) {
  Cell c;
  c.mutable_formula()->mutable_cached_amount()->set_str_amount("foo");
  EXPECT_THAT(PrintCell(c), Eq("foo"));
}

TEST(PrintCell, IntAmount) {
  Cell c;
  c.mutable_formula()->mutable_cached_amount()->set_int_amount(1234);
  EXPECT_THAT(PrintCell(c), Eq("1234"));
}

TEST(PrintCell, DoubleAmount) {
  Cell c;
  c.mutable_formula()->mutable_cached_amount()->set_double_amount(12.34567890);
  EXPECT_THAT(PrintCell(c), Eq("12.35"));
}

TEST(PrintCell, TimestampAmount) {
  Cell c;
  c.mutable_formula()
      ->mutable_cached_amount()
      ->mutable_timestamp_amount()
      ->set_seconds(0);
  EXPECT_THAT(PrintCell(c), Eq("1970-01-01T00:00:00+00:00"));
}

TEST(PrintCell, MoneyAmount) {
  Cell c;

  auto *money =
      c.mutable_formula()->mutable_cached_amount()->mutable_money_amount();
  money->set_currency(Money::USD);

  money->set_dollars(10);
  money->set_cents(10);
  EXPECT_THAT(PrintCell(c), Eq("$10.10"));

  money->set_dollars(0);
  EXPECT_THAT(PrintCell(c), Eq("$0.10"));

  money->set_cents(0);
  EXPECT_THAT(PrintCell(c), Eq("$0.00"));
}

TEST(GridView, ConstructAndPrint) {
  absl::flat_hash_map<XY, Cell> xy_cells{};
  xy_cells[XY(0, 0)] = ToProto<Cell>(R"pb(
      point_location: { row: 0 col: 0 } formula: { cached_amount: { int_amount: 5 } }
  )pb");
  xy_cells[XY(0, 1)] = ToProto<Cell>(R"pb(
      point_location: { row: 1 col: 0 } formula: { cached_amount: { int_amount: 10 } }
  )pb");
  xy_cells[XY(1, 0)] = ToProto<Cell>(R"pb(
      point_location: { row: 0 col: 1 } formula: { cached_amount: { double_amount: 5.0 } }
  )pb");
  xy_cells[XY(1, 1)] = ToProto<Cell>(R"pb(
      point_location: { row: 1 col: 1 } formula: { cached_amount: { double_amount: 10.0 } }
  )pb");

  auto gv = GridView({.height = 2, .width = 2, .offset_x = 0, .offset_y = 0});

  for (const auto &[xy, cell] : xy_cells) {
    gv.Write(xy, &cell);
  }

  std::ostringstream out;
  out << gv;
  EXPECT_THAT(out.str(), Eq(R"(+----+-------+
|  5 |  5.00 |
+----+-------+
| 10 | 10.00 |
+----+-------+
)"));

  std::cout << out.str() << std::endl;
}

//
//
//
//
//
//
//
//
//
//
//
//
//

} // namespace
} // namespace latis
