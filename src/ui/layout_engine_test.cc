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

#include "src/test_utils/test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace ui {
namespace {

using ::testing::DoubleEq;
using ::testing::Eq;
using ::testing::Le;
using ::testing::MockFunction;
using ::testing::Not;
using ::testing::Property;
using ::testing::StrictMock;

TEST(LayoutEngine, Placements) {
  const int w = 8;
  const int half_w = 4;

  auto e = LayoutEngine(/*y=*/10, /*x=*/w);

  EXPECT_THAT(e.Place(1, half_w), Eq(ui::Dimensions{1, 4, 0, 0}));
}

} // namespace
} // namespace ui
} // namespace latis
