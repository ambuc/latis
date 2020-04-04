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
using ::testing::MockFunction;
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
protected:
  const XY A1 = XY::From("A1").ValueOrDie();
  const XY B1 = XY::From("B1").ValueOrDie();
  const XY C1 = XY::From("C1").ValueOrDie();
  Latis latis_;
};

TEST_F(LatisTest, Set) {
  EXPECT_THAT(latis_.Set(A1, "2.0"),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("double_amount: 2.0"))));
}

TEST_F(LatisTest, SetAndGet) {
  EXPECT_THAT(latis_.Set(A1, "2.0"),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("double_amount: 2.0"))));

  EXPECT_THAT(
      latis_.Set(A1, "\"string\""),
      IsOkAndHolds(EqualsProto(ToProto<Amount>("str_amount: \"string\""))));

  EXPECT_THAT(latis_.Set(A1, "2"), IsOk());
  EXPECT_THAT(latis_.Set(B1, "2"), IsOk());
  EXPECT_THAT(latis_.Set(C1, "A1+B1"),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 4"))));
}

TEST_F(LatisTest, SetAndGetAndChange) {
  StrictMock<MockFunction<void(const Cell &)>> update_cb;
  EXPECT_CALL(update_cb, Call).Times(0);

  latis_.RegisterUpdateCallback(update_cb.AsStdFunction());

  EXPECT_THAT(latis_.Set(A1, "2"), IsOk());
  EXPECT_THAT(latis_.Set(B1, "2"), IsOk());
  EXPECT_THAT(latis_.Set(C1, "A1+B1"),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 4"))));

  EXPECT_CALL(update_cb, Call).Times(1);

  // If we update a dependency,
  EXPECT_THAT(latis_.Set(B1, "3"), IsOk());
  // The dependent is autoupdated and will return the correct value at next
  // fetch.
  EXPECT_THAT(latis_.Get(C1),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 5"))));

  // But what if we change C1 so it's no longer dependent on B1?
  // We should be able to update B1 and NOT receive a cb on the update_cb
  // channel.
  EXPECT_CALL(update_cb, Call).Times(0);
  EXPECT_THAT(latis_.Set(C1, "A1"), IsOk());
  EXPECT_THAT(latis_.Set(B1, "4"), IsOk());

  // Now if we do update A1, C1:=A1.
  EXPECT_CALL(update_cb, Call).Times(1);
  EXPECT_THAT(latis_.Set(A1, "1"), IsOk());
  EXPECT_THAT(latis_.Get(C1),
              IsOkAndHolds(EqualsProto(ToProto<Amount>("int_amount: 1"))));
}

} // namespace
} // namespace latis
