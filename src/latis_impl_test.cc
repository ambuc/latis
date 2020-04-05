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
using ::testing::DoubleEq;
using ::testing::Eq;
using ::testing::Le;
using ::testing::MockFunction;
using ::testing::Not;
using ::testing::Property;
using ::testing::StrictMock;

// metadata
TEST(Metadata, TitleAndAuthor) {
  Latis latis;
  latis.SetTitle("t");
  latis.SetAuthor("a");

  LatisMsg latis_msg_output;
  EXPECT_THAT(latis.WriteTo(&latis_msg_output), IsOk());

  EXPECT_THAT(latis_msg_output.metadata().title(), Eq("t"));
  EXPECT_THAT(latis_msg_output.metadata().author(), Eq("a"));
}

TEST(Metadata, CreatedTimeIsSetDuringIngest) {
  Latis latis;

  LatisMsg latis_msg_output;
  EXPECT_THAT(latis.WriteTo(&latis_msg_output), IsOk());

  EXPECT_THAT(latis_msg_output.metadata().created_time().seconds(),
              Le(absl::ToUnixSeconds(absl::Now())));
}

TEST(Metadata, CreatedTimeIsUntouched) {
  const int s = 12345;
  LatisMsg latis_msg_input_with_creation_time;
  latis_msg_input_with_creation_time.mutable_metadata()
      ->mutable_created_time()
      ->set_seconds(s);

  Latis latis(latis_msg_input_with_creation_time);
  EXPECT_THAT(absl::ToUnixSeconds(latis.CreatedTime()), Eq(s));

  LatisMsg latis_msg_output;
  EXPECT_THAT(latis.WriteTo(&latis_msg_output), IsOk());

  EXPECT_THAT(latis_msg_output.metadata().created_time().seconds(), Eq(s));
}

TEST(Metadata, EditedTimeIsEdited) {
  LatisMsg latis_msg_input;

  Latis latis(latis_msg_input);
  EXPECT_THAT(latis.EditedTime(), Le(absl::Now()));

  LatisMsg latis_msg_output;
  EXPECT_THAT(latis.WriteTo(&latis_msg_output), IsOk());

  EXPECT_THAT(latis_msg_output.metadata().edited_time().seconds(),
              Le(absl::ToUnixSeconds(absl::Now())));
}

class LatisTest : public ::testing::Test {
public:
  void SetUp() { latis_.RegisterUpdateCallback(update_cb_.AsStdFunction()); }

protected:
  const XY A1 = XY::From("A1").ValueOrDie();
  const XY B2 = XY::From("B2").ValueOrDie();
  const XY C3 = XY::From("C3").ValueOrDie();
  const XY D4 = XY::From("D4").ValueOrDie();
  Latis latis_;
  StrictMock<MockFunction<void(const Cell &)>> update_cb_;
};

TEST_F(LatisTest, Set) {
  EXPECT_THAT(latis_.Set(A1, "2.0"),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("double_amount: 2.0"))));
}

TEST_F(LatisTest, NoCycle) {
  latis_.Set(A1, "1.0");
  EXPECT_THAT(latis_.Set(A1, "A1"), Not(IsOk()));
}

TEST_F(LatisTest, NoCycleLonger) {
  latis_.Set(A1, "1.0");
  latis_.Set(B2, "A1");
  EXPECT_THAT(latis_.Set(A1, "B2"), Not(IsOk()));
}

TEST_F(LatisTest, SetAndGet) {
  EXPECT_THAT(latis_.Set(A1, "2.0"),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("double_amount: 2.0"))));

  EXPECT_THAT(
      latis_.Set(A1, "\"string\""),
      IsOkAndHolds(EqualsProto(ToProto<Amount>("str_amount: \"string\""))));

  latis_.Set(A1, "2");
  latis_.Set(B2, "2");
  EXPECT_THAT(latis_.Set(C3, "A1+B2"),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 4"))));
}

TEST_F(LatisTest, ChangeUnderlyingValue) {
  EXPECT_CALL(update_cb_, Call).Times(0);

  latis_.Set(A1, "2");
  latis_.Set(B2, "2");
  EXPECT_THAT(latis_.Set(C3, "A1+B2"),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 4"))));

  EXPECT_CALL(update_cb_, Call).Times(1);

  // If we update a dependency,
  latis_.Set(B2, "3");
  // The dependent is autoupdated and will return the correct value at next
  // fetch.
  ASSERT_THAT(latis_.Get(C3),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 5"))));
}

TEST_F(LatisTest, BreakDependency) {
  EXPECT_CALL(update_cb_, Call).Times(0);

  latis_.Set(A1, "2");
  latis_.Set(B2, "2");
  EXPECT_THAT(latis_.Set(C3, "A1+B2"),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 4"))));

  // But what if we change C3 so it's no longer dependent on B2?
  // We should be able to update B2 and NOT receive a cb on the update_cb_
  // channel.
  EXPECT_CALL(update_cb_, Call).Times(0);
  latis_.Set(C3, "A1");
  latis_.Set(B2, "4");

  // Now if we do update A1, C3:=A1.
  EXPECT_CALL(update_cb_, Call).Times(1);
  latis_.Set(A1, "1");
  ASSERT_THAT(latis_.Get(C3),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 1"))));
}

TEST_F(LatisTest, DependencyTwoHops) {
  latis_.Set(A1, "1.1");
  latis_.Set(B2, "2.2");
  EXPECT_THAT(latis_.Set(C3, "A1+B2"),
              IsOkAndHolds(Property(&Amount::double_amount, DoubleEq(3.3))));
  EXPECT_THAT(latis_.Set(D4, "C3*2"),
              IsOkAndHolds(Property(&Amount::double_amount, DoubleEq(6.6))));

  // Two updates, one for C3 and one for D4.
  EXPECT_CALL(update_cb_, Call).Times(2);
  latis_.Set(A1, "1.2");
  EXPECT_THAT(latis_.Get(C3),
              IsOkAndHolds(Property(&Amount::double_amount, DoubleEq(3.4))));
  EXPECT_THAT(latis_.Get(D4),
              IsOkAndHolds(Property(&Amount::double_amount, DoubleEq(6.8))));
}

} // namespace
} // namespace latis
