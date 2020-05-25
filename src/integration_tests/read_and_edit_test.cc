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

#include "src/integration_tests/integration_test_base.h"

#include "absl/strings/str_format.h"
#include "proto/latis_msg.pb.h"
#include "src/xy.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace test {
namespace {

using ::testing::AllOf;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::MatchesRegex;

MATCHER_P2(HasCell, cell, value, "") {
  //       C
  //     ┼─────────────┼
  //   R │ V           │
  //     ┼─────────────┼
  std::string col = cell.ToColumnLetter();
  int row = cell.Y() + 1;
  return Matches(MatchesRegex(absl::StrFormat(".*\n"
                                              ".*%s[^\n]*\n"
                                              ".*"
                                              ".*┼[─]*┼.*\n"
                                              ".*%d │ %s.*│.*\n"
                                              ".*┼[─]*┼.*\n"
                                              ".*",
                                              col, row, value)))(arg);
}

TEST_F(IntegrationTestBase, ReadMetadata) {
  LatisMsg msg;
  msg.mutable_metadata()->set_title("foo");
  msg.mutable_metadata()->set_author("bar");

  SendLatisMsg(msg);

  EXPECT_THAT(Dump(), AllOf(HasSubstr("Title: foo"), HasSubstr("Author: bar")));
}

TEST_F(IntegrationTestBase, A1Has4) {
  const auto loc = XY::From("A1").ValueOrDie();

  LatisMsg msg;
  auto *cell = msg.add_cells();
  *(cell->mutable_point_location()) = loc.ToPointLocation();
  auto *formula = cell->mutable_formula();
  formula->mutable_expression()->mutable_value()->set_int_amount(4);
  formula->mutable_cached_amount()->set_int_amount(4);

  SendLatisMsg(msg);

  EXPECT_THAT(Dump(), HasCell(loc, "4"));
}

} // namespace
} // namespace test
} // namespace latis
