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

#include "src/latis_impl.h"

#include "absl/time/time.h"
#include "google/protobuf/text_format.h"
#include "src/display_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace {

using ::google::protobuf::TextFormat;
using ::google::protobuf::util::StatusOr;
using ::testing::Eq;
using ::testing::Le;

class GetTest : public ::testing::Test {
public:
  void SetUp() override {
    Cell *cell = latis_msg_input_.add_cells();
    *cell->mutable_point_location() =
        XY::From("A1").ValueOrDie().ToPointLocation();
  }

  Cell *MutableCell() { return latis_msg_input_.mutable_cells(0); }

  Amount *MutableAmount() {
    return latis_msg_input_.mutable_cells(0)->mutable_amount();
  }

  void TearDown() override {
    Latis latis(latis_msg_input_);

    EXPECT_EQ(latis.Get(XY::From("A1").ValueOrDie())->Export().DebugString(),
              MutableCell()->DebugString());
  }

protected:
  LatisMsg latis_msg_input_;
};

TEST(GetTestEmpty, GetAmountEmpty) {
  Latis latis;
  EXPECT_EQ(latis.Get(XY(0, 0)), nullptr);
}

TEST_F(GetTest, GetAmountSimpleString) {
  MutableAmount()->set_str_amount("foo");
}

TEST_F(GetTest, GetAmountInt) { MutableAmount()->set_int_amount(1234); }

TEST_F(GetTest, GetAmountMoneyZero) {
  TextFormat::ParseFromString(
      R"pb(money_amount {dollars: 0 cents: 0 currency: USD })pb",
      MutableAmount());
}

TEST_F(GetTest, GetAmountMoneyNonZero) {
  TextFormat::ParseFromString(
      R"pb(money_amount {dollars: 12 cents: 34 currency: USD })pb",
      MutableAmount());
}

class SetTest : public ::testing::Test {
protected:
  Latis latis_;
  XY a1_{XY::From("A1").ValueOrDie()};
};

// TEST_F(SetTest, SetInt) {
//   latis_.Set(CellObj::From(a1_, "1").ValueOrDie());
//   EXPECT_EQ(latis_.Print(a1_), "1");
//
//   latis_.Set(CellObj::From(a1_, "0").ValueOrDie());
//   EXPECT_EQ(latis_.Print(a1_), "0");
//
//   latis_.Set(CellObj::From(a1_, "-2").ValueOrDie());
//   EXPECT_EQ(latis_.Print(a1_), "-2");
// }
//
// TEST_F(SetTest, SetDouble) {
//   latis_.Set(CellObj::From(a1_, "1.2345").ValueOrDie());
//   EXPECT_EQ(latis_.Print(a1_), "1.23");
//
//   latis_.Set(CellObj::From(a1_, "0.00").ValueOrDie());
//   EXPECT_EQ(latis_.Print(a1_), "0.00");
//
//   latis_.Set(CellObj::From(a1_, "-1.2345").ValueOrDie());
//   EXPECT_EQ(latis_.Print(a1_), "-1.23");
// }
//
// TEST_F(SetTest, SetMoney) {
//   latis_.Set(CellObj::From(a1_, "$1.23").ValueOrDie());
//   EXPECT_EQ(latis_.Print(a1_), "$1.23");
// }

//
// class FormulaTest : public ::testing::Test {
// public:
//   void SetUp() override {}
//
// protected:
//   Latis latis_;
//   XY a1_{XY::From("A1").ValueOrDie()};
//   XY a2_{XY::From("A2").ValueOrDie()};
//   XY a3_{XY::From("A3").ValueOrDie()};
// };
//
// TEST_F(FormulaTest, TestSetFormula) {
//   // A1 = 1
//   latis_.Set(CellObj::From(a1_, "1").ValueOrDie());
//   // A2 = 2
//   latis_.Set(CellObj::From(a2_, "2").ValueOrDie());
//   // A3 = A1 + A2
//   latis_.Set(CellObj::From(a3_, "=A1+A2").ValueOrDie());
//
//   EXPECT_EQ(latis_.Print(a3_), "3");
// }
//
} // namespace
} // namespace latis
