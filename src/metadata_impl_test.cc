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

#include "src/metadata_impl.h"

#include "google/protobuf/text_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace {

using ::google::protobuf::TextFormat;
using ::google::protobuf::util::StatusOr;
using ::testing::Eq;
using ::testing::Le;

TEST(MetadataImpl, TitleAndAuthor) {
  MetadataImpl mi;
  mi.SetTitle("t");
  mi.SetAuthor("a");

  LatisMsg latis_msg_output;
  mi.WriteTo(&latis_msg_output);

  EXPECT_THAT(latis_msg_output.metadata().title(), Eq("t"));
  EXPECT_THAT(latis_msg_output.metadata().author(), Eq("a"));
}

TEST(MetadataImpl, CreatedTimeIsSetDuringIngest) {
  MetadataImpl mi;

  LatisMsg latis_msg_output;
  mi.WriteTo(&latis_msg_output);

  EXPECT_THAT(latis_msg_output.metadata().created_time().seconds(),
              Le(absl::ToUnixSeconds(absl::Now())));
}

TEST(MetadataImpl, CreatedTimeIsUntouched) {
  const int s = 12345;
  LatisMsg latis_msg_input_with_creation_time;
  latis_msg_input_with_creation_time.mutable_metadata()
      ->mutable_created_time()
      ->set_seconds(s);

  MetadataImpl mi(latis_msg_input_with_creation_time.metadata());
  EXPECT_THAT(absl::ToUnixSeconds(mi.CreatedTime()), Eq(s));

  LatisMsg latis_msg_output;
  mi.WriteTo(&latis_msg_output);

  EXPECT_THAT(latis_msg_output.metadata().created_time().seconds(), Eq(s));
}

TEST(MetadataImpl, EditedTimeIsEdited) {
  LatisMsg latis_msg_input;

  MetadataImpl mi(latis_msg_input.metadata());
  EXPECT_THAT(mi.EditedTime(), Le(absl::Now()));

  LatisMsg latis_msg_output;
  mi.WriteTo(&latis_msg_output);

  EXPECT_THAT(latis_msg_output.metadata().edited_time().seconds(),
              Le(absl::ToUnixSeconds(absl::Now())));
}

} // namespace
} // namespace latis
