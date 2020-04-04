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

#ifndef SRC_XY_H_
#define SRC_XY_H_

#include "proto/latis_msg.pb.h"

#include "absl/hash/hash.h"
#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/statusor.h"

namespace latis {

// XY is the lingua franca.
//
//  (PointLocation)
//      Pl <=> XY <=> A1
//             ^
//             |
//             v
//            Cl
//         (Column)

class XY {
public:
  // ctors
  XY() : x_(0), y_(0) {}
  XY(int x, int y) : x_(x), y_(y) {}
  static XY From(std::tuple<int, int> tuple);
  static XY From(PointLocation pl);
  static ::google::protobuf::util::StatusOr<XY> From(std::string_view a1);

  // exports
  PointLocation ToPointLocation() const;
  std::string ToA1() const;
  std::string ToColumnLetter() const;

  // utils
  static ::google::protobuf::util::StatusOr<int>
  ColumnLetterToInteger(std::string_view s);
  static std::string IntegerToColumnLetter(int i);

  friend bool operator==(const XY &, const XY &);
  friend bool operator<(const XY &, const XY &);

  // Necessary for absl::flat_hash_*
  template <typename H> friend H AbslHashValue(H h, const XY &xy) {
    return H::combine(std::move(h), xy.x_, xy.y_);
  }

private:
  int x_;
  int y_;
};

inline bool operator==(const XY &lhs, const XY &rhs) {
  return lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_;
}
// Necessary for std::multimap
inline bool operator<(const XY &lhs, const XY &rhs) {
  return lhs.x_ < rhs.x_ && lhs.y_ < rhs.y_;
}

} // namespace latis

#endif // SRC_XY_H_
