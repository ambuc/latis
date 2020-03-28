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

#include "src/utils/status_macros.h"

#include "proto/latis_msg.pb.h"
#include "src/test_utils/test_utils.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/status_macros.h"
#include "google/protobuf/stubs/statusor.h"

namespace latis {
namespace {

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;
using ::google::protobuf::util::error::OK;

TEST(ReturnIfError, Ok) {
  EXPECT_THAT(
      []() -> Status {
        // Doesn't return.
        RETURN_IF_ERROR_([]() -> Status { return Status(OK, ""); }());

        return Status(OK, "");
      }(),
      IsOk());
}

TEST(ReturnIfError, NotOk) {
  EXPECT_THAT(
      []() -> Status {
        // Returns.
        RETURN_IF_ERROR_(
            []() -> Status { return Status(INVALID_ARGUMENT, ""); }());

        return Status(OK, "");
      }(),
      ::testing::Not(IsOk()));
}

TEST(AssignOrReturn, Ok) {
  EXPECT_THAT(
      []() -> StatusOr<int> {
        // Doesn't return.
        int val;
        ASSIGN_OR_RETURN_(val, []() -> StatusOr<int> { return 2; }());
        return val;
      }(),
      IsOkAndHolds(2));
}

TEST(AssignOrReturn, NotOk) {
  EXPECT_THAT(
      []() -> StatusOr<int> {
        // Does return.
        int val;
        ASSIGN_OR_RETURN_(val, []() -> StatusOr<int> {
          return Status(INVALID_ARGUMENT, "");
        }());
        return val;
      }(),
      ::testing::Not(IsOk()));
}

} // namespace
} // namespace latis
