/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_TEST_UTILS_TEST_UTILS_H_
#define SRC_TEST_UTILS_TEST_UTILS_H_

#include "google/protobuf/text_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <google/protobuf/util/message_differencer.h>

namespace latis {

using ::testing::Matches;

// Matchers for
// https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/stubs/status.h.
//
MATCHER(IsOk, "") {
  // *result_listener << arg.status();
  return arg.ok();
}

MATCHER_P(IsOkAndHolds, m, "") {
  *result_listener << arg.status();
  return arg.ok() && Matches(m)(arg.ValueOrDie());
}

MATCHER_P(EqualsProto, p, "") {
  // TODO(ambuc): get this working.
  // std::string msg;
  // ::google::protobuf::util::MessageDifferencer::ReportDifferencesToString(&msg);

  // *result_listener << msg;

  return ::google::protobuf::util::MessageDifferencer::Equals(arg, p);
}

// Useful in tests. Cribbed partially from
// https://github.com/google/shell-encryption/blob/master/testing/status_testing.h#L22-L25.
#define ASSERT_OK_AND_ASSIGN(val, expr)                                        \
  ({                                                                           \
    auto _expr_result = (expr);                                                \
    EXPECT_THAT(expr, IsOk());                                                 \
    val = expr.ValueOrDie();                                                   \
  })

template <typename T> //
T ToProto(const std::string &s) {
  T t;
  ::google::protobuf::TextFormat::ParseFromString(s, &t);
  return t;
}

} // namespace latis

#endif // SRC_TEST_UTILS_TEST_UTILS_H_
