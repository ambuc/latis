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

#ifndef SRC_FORMULA_FUNCTIONS_H_
#define SRC_FORMULA_FUNCTIONS_H_

#include "proto/latis_msg.pb.h"

#include "absl/types/span.h"
#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/status_macros.h"
#include "google/protobuf/stubs/statusor.h"
#include <google/protobuf/util/message_differencer.h>

namespace latis {
namespace formula {

using ::google::protobuf::Timestamp;
using ::google::protobuf::util::StatusOr;

// Timestamp
StatusOr<bool> operator<=(const Timestamp &lhs, const Timestamp &rhs);
StatusOr<Timestamp> operator+(const Timestamp &lhs, const Timestamp &rhs);
Timestamp operator-(const Timestamp &arg);

// Money
StatusOr<bool> operator<=(const Money &lhs, const Money &rhs);
StatusOr<Money> operator+(const Money &lhs, const Money &rhs);
Money operator-(const Money &arg);
StatusOr<Money> operator*(const Money &lhs, const Money &rhs);
StatusOr<Money> operator/(const Money &lhs, const Money &rhs);

// Amount (operating on numeric)
StatusOr<bool> operator<=(const Amount &lhs, const Amount &rhs);
StatusOr<bool> operator==(const Amount &lhs, const Amount &rhs);
StatusOr<Amount> operator+(const Amount &lhs, const Amount &rhs);
StatusOr<Amount> operator-(const Amount &arg);
StatusOr<Amount> operator*(const Amount &lhs, const Amount &rhs);
StatusOr<Amount> operator/(const Amount &lhs, const Amount &rhs);
// Amount (operating on bool)
StatusOr<Amount> operator&&(const Amount &lhs, const Amount &rhs);
StatusOr<Amount> operator||(const Amount &lhs, const Amount &rhs);
StatusOr<Amount> operator!(const Amount &arg);

// Templated operators for protos (Money, Timestamp, etc.)
template <typename T> //
StatusOr<T> operator-(const T &lhs, const T &rhs) {
  if (const StatusOr<T> maybe_neg_rhs = -rhs; !maybe_neg_rhs.ok()) {
    return maybe_neg_rhs.status();
  } else {
    return lhs + maybe_neg_rhs.ValueOrDie();
  }
}
template <typename T> //
StatusOr<bool> operator==(const T &lhs, const T &rhs) {
  return ::google::protobuf::util::MessageDifferencer::Equals(lhs, rhs);
}
template <typename T> //
StatusOr<bool> operator!=(const T &lhs, const T &rhs) {
  const auto eq_or_status = (lhs == rhs);
  if (!eq_or_status.ok()) {
    return eq_or_status.status();
  }
  return !(eq_or_status.ValueOrDie());
}
template <typename T> //
StatusOr<bool> operator<(const T &lhs, const T &rhs) {
  const auto leq_or_status = (lhs <= rhs);
  if (!leq_or_status.ok()) {
    return leq_or_status.status();
  }
  const auto neq_or_status = (lhs != rhs);
  if (!neq_or_status.ok()) {
    return neq_or_status.status();
  }
  return leq_or_status.ValueOrDie() && neq_or_status.ValueOrDie();
}
template <typename T> //
StatusOr<bool> operator>(const T &lhs, const T &rhs) {
  return !(lhs <= rhs);
}
template <typename T> //
StatusOr<bool> operator>=(const T &lhs, const T &rhs) {
  return lhs > rhs || lhs == rhs;
}

// TODO(ambuc): pow, mod, <, >,

} // namespace formula
} // namespace latis

#endif // SRC_FORMULA_FUNCTIONS_H_
