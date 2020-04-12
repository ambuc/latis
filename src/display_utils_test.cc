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
  EXPECT_THAT(PrintCell(c), Eq("'foo'"));
}

TEST(PrintCell, IntAmount) {
  Cell c;
  c.mutable_formula()->mutable_cached_amount()->set_int_amount(1234);
  EXPECT_THAT(PrintCell(c, {.width = 4}), Eq("1234"));
}

TEST(PrintCell, DoubleAmount) {
  Cell c;
  c.mutable_formula()->mutable_cached_amount()->set_double_amount(12.34567890);
  EXPECT_THAT(PrintCell(c, {.width = 5, .double_precision = 2}), Eq("12.35"));
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

class GridViewTest : public ::testing::Test {
public:
  //      0     1      2                3                 4
  //   +----+--------+   +---------------------------+--------+
  // 0 |  5 |   5.00 |   |                     "foo" | $10.10 |
  //   +----+--------+   +---------------------------+--------+
  // 1 | 10 |  10.00 |   | 1970-01-01T00:00:00+00:00 |   TRUE |
  //   +----+--------+   +---------------------------+--------+
  // 2 |    | 100.00 |   | 1970-01-01T00:00:00+00:00 |   TRUE |
  //   +----+--------+   +---------------------------+--------+
  const std::vector<Cell> cell_protos_{
      ToProto<Cell>(
          R"(point_location: { row: 0 col: 0 } formula: { cached_amount: { int_amount: 5 } })"),
      ToProto<Cell>(
          R"(point_location: { row: 1 col: 0 } formula: { cached_amount: { int_amount: 10 } })"),
      ToProto<Cell>(
          R"(point_location: { row: 0 col: 1 } formula: { cached_amount: { double_amount: 5.0 } })"),
      ToProto<Cell>(
          R"(point_location: { row: 1 col: 1 } formula: { cached_amount: { double_amount: 10.0 } })"),
      ToProto<Cell>(
          R"(point_location: { row: 2 col: 1 } formula: { cached_amount: { double_amount: 100.0 } })"),
      //
      ToProto<Cell>(
          R"(point_location: { row: 0 col: 3 } formula: { cached_amount: { str_amount: "foo" } })"),
      ToProto<Cell>(
          R"(point_location: { row: 0 col: 4 } formula: { cached_amount: { 
               money_amount: { currency: USD dollars: 10 cents: 10 }
             } })"),
      ToProto<Cell>(
          R"(point_location: { row: 1 col: 3 } formula: { cached_amount: { 
               timestamp_amount: { seconds: 1451675045 }
             } })"),
      ToProto<Cell>(
          R"(point_location: { row: 1 col: 4 } formula: { cached_amount: { bool_amount: true } })"),
  };

  std::string ToStr(GridView gv) {
    for (const auto &p : cell_protos_) {
      gv.Write(XY::From(p.point_location()), &p);
    }

    std::ostringstream out;
    out << gv;
    return out.str();
  }
};

TEST_F(GridViewTest, ConstructAndPrintAll) {
  auto gv = GridView({
      .height = 2,
      .width = 2,
      .double_precision = 2,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("+----+-------+\n"
                            "|  5 |  5.00 |\n"
                            "+----+-------+\n"
                            "| 10 | 10.00 |\n"
                            "+----+-------+\n"));
}

TEST_F(GridViewTest, ConstructAndPrintSome1) {
  auto gv = GridView({
      .height = 2,
      .width = 1,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("+----+\n"
                            "|  5 |\n"
                            "+----+\n"
                            "| 10 |\n"
                            "+----+\n"));
}

TEST_F(GridViewTest, ConstructAndPrintSome2) {
  auto gv = GridView({
      .height = 1,
      .width = 2,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("+---+------+\n"
                            "| 5 | 5.00 |\n"
                            "+---+------+\n"));
}

TEST_F(GridViewTest, ConstructAndPrintSome3) {
  auto gv = GridView({
      .height = 2,
      .width = 1,
      .offset_x = 1,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("+-------+\n"
                            "|  5.00 |\n"
                            "+-------+\n"
                            "| 10.00 |\n"
                            "+-------+\n"));
}

TEST_F(GridViewTest, ConstructAndPrintSome4) {
  auto gv = GridView({
      .height = 1,
      .width = 2,
      .offset_y = 1,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("+----+-------+\n"
                            "| 10 | 10.00 |\n"
                            "+----+-------+\n"));
}

TEST_F(GridViewTest, ConstructAndPrintSome5) {
  auto gv = GridView({
      .height = 3,
      .width = 2,
      .double_precision = 2,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("+----+--------+\n"
                            "|  5 |   5.00 |\n"
                            "+----+--------+\n"
                            "| 10 |  10.00 |\n"
                            "+----+--------+\n"
                            "|    | 100.00 |\n"
                            "+----+--------+\n"));
}

TEST_F(GridViewTest, ConstructAndPrintSecondSetAll) {
  auto gv = GridView({
      .height = 2,
      .width = 2,
      .offset_x = 3,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("+---------------------------+--------+\n"
                            "|                     'foo' | $10.10 |\n"
                            "+---------------------------+--------+\n"
                            "| 2016-01-01T19:04:05+00:00 |   True |\n"
                            "+---------------------------+--------+\n"));
}

TEST_F(GridViewTest, ConstructAndPrintSecondSetSome1) {
  auto gv = GridView({
      .height = 2,
      .width = 1,
      .offset_x = 3,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("+---------------------------+\n"
                            "|                     'foo' |\n"
                            "+---------------------------+\n"
                            "| 2016-01-01T19:04:05+00:00 |\n"
                            "+---------------------------+\n"));
}

TEST_F(GridViewTest, ConstructAndPrintSecondSetSome2) {
  auto gv = GridView({
      .height = 2,
      .width = 1,
      .offset_x = 4,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("+--------+\n"
                            "| $10.10 |\n"
                            "+--------+\n"
                            "|   True |\n"
                            "+--------+\n"));
}

TEST_F(GridViewTest, BoxDrawing) {
  auto gv = GridView({
      .height = 2,
      .width = 2,
      .double_precision = 2,
      .border_style = internal::BorderStyle::kBoxDrawing,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("┌────┬───────┐\n"
                            "│  5 │  5.00 │\n"
                            "├────┼───────┤\n"
                            "│ 10 │ 10.00 │\n"
                            "└────┴───────┘\n"));
}

TEST_F(GridViewTest, FancyBoxDrawing) {
  auto gv = GridView({
      .height = 2,
      .width = 2,
      .double_precision = 2,
      .border_style = internal::BorderStyle::kFancyBoxDrawing,
      .show_coordinates = false,
  });
  EXPECT_THAT(ToStr(gv), Eq("╔════╤═══════╗\n"
                            "║  5 │  5.00 ║\n"
                            "╟────┼───────╢\n"
                            "║ 10 │ 10.00 ║\n"
                            "╚════╧═══════╝\n"));
}

TEST_F(GridViewTest, FancyBoxDrawingWithCoordinates) {
  auto gv = GridView({
      .height = 2,
      .width = 2,
      .double_precision = 2,
      .border_style = internal::BorderStyle::kFancyBoxDrawing,
      .show_coordinates = true,
  });
  EXPECT_THAT(ToStr(gv), Eq("      A       B  \n"
                            "   ╔════╤═══════╗\n"
                            " 1 ║  5 │  5.00 ║\n"
                            "   ╟────┼───────╢\n"
                            " 2 ║ 10 │ 10.00 ║\n"
                            "   ╚════╧═══════╝\n"));
}

} // namespace
} // namespace latis
