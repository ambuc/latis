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
#include "src/test_utils/test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace {

using ::google::protobuf::TextFormat;
using ::google::protobuf::util::StatusOr;
using ::testing::Eq;
using ::testing::Le;

TEST(Latis, SetAndGet) {
  const XY A1 = XY::From("A1").ValueOrDie();
  const XY B1 = XY::From("B1").ValueOrDie();
  const XY C1 = XY::From("C1").ValueOrDie();

  Latis latis;

  EXPECT_OK(latis.Set(A1, "2.0"));
  EXPECT_THAT(latis.Get(A1),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("double_amount: 2.0"))));

  EXPECT_OK(latis.Set(A1, "\"string\""));
  EXPECT_THAT(latis.Get(A1), IsOkAndHolds(EqualsProto(
                                 ToProto<Amount>("str_amount: \"string\""))));

  EXPECT_OK(latis.Set(A1, "2"));
  EXPECT_OK(latis.Set(B1, "2"));
  EXPECT_OK(latis.Set(C1, "A1+B1"));
  EXPECT_THAT(latis.Get(C1),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 4"))));
}

} // namespace
} // namespace latis
