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

#ifndef SRC_LATIS_IMPL_H_
#define SRC_LATIS_IMPL_H_

#include "src/latis_interface.h"

#include "proto/latis_msg.pb.h"
#include "src/display_utils.h"
#include "src/formula/common.h"
#include "src/formula/formula.h"
#include "src/metadata_interface.h"
#include "src/xy.h"

#include "absl/base/thread_annotations.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"

namespace latis {

class Latis : public LatisInterface {
public:
  Latis();
  Latis(const LatisMsg &sheet);

  // Getters
  ::google::protobuf::util::StatusOr<Amount> Get(XY xy) override;
  std::string Print(XY xy) const override;

  // Setters
  ::google::protobuf::util::Status Set(XY xy, std::string_view input) override;

  // Export
  LatisMsg Write() const override;

  // Kept structs
  MetadataInterface *Metadata() override { return metadata_.get(); }

private:
  // Thread-compatible but not thread-safe.
  class CellObj {
  public:
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

    static ::google::protobuf::util::StatusOr<CellObj>
    From(XY xy, std::string_view input, formula::LookupFn *lookup_fn) {
      Amount a;
      ASSIGN_OR_RETURN_(a, formula::Parse(input, *lookup_fn));
      return CellObj(xy, a);
    }

    // Getter
    XY Xy() const { return XY::From(cell_.point_location()); }

    Amount GetAmount() const {
      // proto Cells always have one or the other.
      if (cell_.has_amount()) {
        return cell_.amount();
      } else if (cell_.has_formula()) {
        return cell_.formula().cached_amount();
      }
      throw 1;
    }

    // Export
    Cell Export() const { return cell_; }

    // Print
    std::string Print() const { return PrintCell(cell_); }

  private:
    Cell cell_;
  };

  void UpdateEditTime() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    absl::MutexLock l(&mu_);
    metadata_->UpdateEditTime();
  }

  formula::LookupFn *GetLookupFn();

  mutable absl::Mutex mu_;

  std::unique_ptr<MetadataInterface> metadata_ ABSL_GUARDED_BY(mu_);
  absl::flat_hash_map<XY, CellObj> cells_ ABSL_GUARDED_BY(mu_);
  std::multimap<XY, XY> edges_ ABSL_GUARDED_BY(mu_);
};

} // namespace latis

#endif // SRC_LATIS_IMPL_H_
