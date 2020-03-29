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

namespace latis {
namespace formula {

using ::google::protobuf::Timestamp;
using ::google::protobuf::util::StatusOr;

// Timestamp
bool operator<(const Timestamp &lhs, const Timestamp &rhs);
bool operator>(const Timestamp &lhs, const Timestamp &rhs);
bool operator==(const Timestamp &lhs, const Timestamp &rhs);
bool operator<=(const Timestamp &lhs, const Timestamp &rhs);
bool operator>=(const Timestamp &lhs, const Timestamp &rhs);
StatusOr<Timestamp> operator+(const Timestamp &lhs, const Timestamp &rhs);
StatusOr<Timestamp> operator-(const Timestamp &lhs, const Timestamp &rhs);

// Money
bool operator<(const Money &lhs, const Money &rhs);
bool operator>(const Money &lhs, const Money &rhs);
bool operator==(const Money &lhs, const Money &rhs);
bool operator<=(const Money &lhs, const Money &rhs);
bool operator>=(const Money &lhs, const Money &rhs);
StatusOr<Money> operator+(const Money &lhs, const Money &rhs);
StatusOr<Money> operator-(const Money &lhs, const Money &rhs);

// Amount
StatusOr<Amount> operator+(const Amount &lhs, const Amount &rhs);
StatusOr<Amount> operator-(const Amount &lhs, const Amount &rhs);
StatusOr<Amount> operator&&(const Amount &lhs, const Amount &rhs);
StatusOr<Amount> operator||(const Amount &lhs, const Amount &rhs);
StatusOr<Amount> operator!(const Amount &arg);

} // namespace formula
} // namespace latis

#endif // SRC_FORMULA_FUNCTIONS_H_
