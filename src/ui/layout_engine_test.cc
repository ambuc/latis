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

#include "src/ui/layout_engine.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace ui {
namespace {

using ::testing::AllOf;
using ::testing::DoubleEq;
using ::testing::Eq;
using ::testing::Field;
using ::testing::Le;
using ::testing::MockFunction;
using ::testing::Not;
using ::testing::Optional;
using ::testing::StrictMock;

class LayoutEngineTestBase : public ::testing::Test {
public:
  LayoutEngineTestBase() : h_(10), w_(8), e_(/*y=*/h_, /*x=*/w_) {}

protected:
  const int h_;
  const int w_;
  LayoutEngine e_;
};

TEST_F(LayoutEngineTestBase, TooTall) {
  // too tall
  EXPECT_EQ(e_.PlaceTL(h_ + 1, 1), absl::nullopt);
  // just right
  EXPECT_NE(e_.PlaceTL(h_, 1), absl::nullopt);
}

TEST_F(LayoutEngineTestBase, TooWide) {
  // too wide
  EXPECT_EQ(e_.PlaceTL(1, w_ + 1), absl::nullopt);
  // just right
  EXPECT_NE(e_.PlaceTL(1, w_), absl::nullopt);
}

TEST_F(LayoutEngineTestBase, Placements) {

  {
    // AAAA
    Dimensions d = e_.PlaceTL(1, 4).value();
    EXPECT_EQ(d.nlines, 1);
    EXPECT_EQ(d.ncols, 4);
    EXPECT_EQ(d.begin_y, 0);
    EXPECT_EQ(d.begin_x, 0);
  }

  {
    // AAAABBBB
    Dimensions d = e_.PlaceTL(1, 4).value();
    EXPECT_EQ(d.nlines, 1);
    EXPECT_EQ(d.ncols, 4);
    EXPECT_EQ(d.begin_y, 0);
    EXPECT_EQ(d.begin_x, 4);
  }

  {
    // AAAABBBB
    // CCC
    Dimensions d = e_.PlaceTL(1, 3).value();
    EXPECT_EQ(d.nlines, 1);
    EXPECT_EQ(d.ncols, 3);
    EXPECT_EQ(d.begin_y, 1);
    EXPECT_EQ(d.begin_x, 0);
  }

  {
    // AAAABBBB
    // CCCDDD
    //    DDD
    Dimensions d = e_.PlaceTL(2, 3).value();
    EXPECT_EQ(d.nlines, 2);
    EXPECT_EQ(d.ncols, 3);
    EXPECT_EQ(d.begin_y, 1);
    EXPECT_EQ(d.begin_x, 3);
  }

  {
    // AAAABBBB
    // CCCDDD
    // EEEDDD
    // EEE
    Dimensions d = e_.PlaceTL(2, 3).value();
    EXPECT_EQ(d.nlines, 2);
    EXPECT_EQ(d.ncols, 3);
    EXPECT_EQ(d.begin_y, 2);
    EXPECT_EQ(d.begin_x, 0);
  }

  {
    // AAAABBBB
    // CCCDDDFF
    // EEEDDDFF
    // EEE
    Dimensions d = e_.PlaceTL(2, 2).value();
    EXPECT_EQ(d.nlines, 2);
    EXPECT_EQ(d.ncols, 2);
    EXPECT_EQ(d.begin_y, 1);
    EXPECT_EQ(d.begin_x, 6);
  }

  {
    // AAAABBBB
    // CCCDDDFF
    // EEEDDDFF
    // EEEGGG
    //    GGG
    Dimensions d = e_.PlaceTL(2, 3).value();
    EXPECT_EQ(d.nlines, 2);
    EXPECT_EQ(d.ncols, 3);
    EXPECT_EQ(d.begin_y, 3);
    EXPECT_EQ(d.begin_x, 3);
  }
}

} // namespace
} // namespace ui
} // namespace latis
