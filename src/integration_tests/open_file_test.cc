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
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/time/clock.h"
#include "proto/latis_msg.pb.h"
#include "src/utils/io.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace test {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

TEST_F(IntegrationTestBase, TryGetStdoutFromCommand) {
  const auto filename = "src/test_utils/example.textproto";
  Send(absl::StrFormat("src/latis --textproto_input %s", filename));

  const auto file_contents =
      latis::FromTextproto<LatisMsg>(filename).ValueOrDie();

  auto d = Dump();

  EXPECT_EQ(file_contents.metadata().title(), "foo");
  EXPECT_THAT(d, HasSubstr("Title: foo"));

  EXPECT_EQ(file_contents.metadata().author(), "bar");
  EXPECT_THAT(d, HasSubstr("Author: bar"));
}

} // namespace
} // namespace test
} // namespace latis
