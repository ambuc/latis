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

#ifndef SRC_CELL_IMPL_H_
#define SRC_CELL_IMPL_H_

#include "src/cell_interface.h"

#include "proto/latis_msg.pb.h"
#include "src/display_utils.h"
#include "src/xy.h"

#include "google/protobuf/stubs/statusor.h"

namespace latis {

// Thread-compatible but not thread-safe.
// Wrapper around proto latis_msgb Cell.
class CellObj : public CellInterface {
public:
  // Ctors
  CellObj() {}

  explicit CellObj(Cell cell) : cell_(cell) {}

  CellObj(XY xy, Amount amount) {
    *cell_.mutable_point_location() = xy.ToPointLocation();
    *cell_.mutable_amount() = amount;
  }

  CellObj(XY xy, Expression expression) {
    *cell_.mutable_point_location() = xy.ToPointLocation();
    *cell_.mutable_formula()->mutable_expression() = expression;
    // TODO cache amount
    // *cell_.mutable_formula().set_cached_amount() = amount;
  }

  // Getter
  XY Xy() const override { return XY::From(cell_.point_location()); }

  Amount GetAmount() const override {
    // proto Cells always have one or the other.
    if (cell_.has_amount()) {
      return cell_.amount();
    } else if (cell_.has_formula()) {
      return cell_.formula().cached_amount();
    }
    throw 1;
  }

  // Export
  Cell Export() const override { return cell_; }

  // Print
  virtual std::string Print() const { return PrintCell(cell_); }

  // Factory methods
  // static ::google::protobuf::util::StatusOr<CellObj>
  // FromAmount(XY xy, std::string_view input);

  // static ::google::protobuf::util::StatusOr<CellObj>
  // FromFormula(XY xy, std::string input);

  // static ::google::protobuf::util::StatusOr<CellObj> From(XY xy,
  //                                                         std::string input);

private:
  Cell cell_;
};

} // namespace latis

#endif // SRC_CELL_IMPL_H_
