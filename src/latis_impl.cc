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

#include "src/display_utils.h"
#include "src/metadata_impl.h"

#include "absl/memory/memory.h"

namespace latis {

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;

Latis::Latis()
    : lookup_fn_([&](XY xy) -> absl::optional<Amount> {
        if (const auto it = cells_.find(xy); it == cells_.end()) {
          return absl::nullopt;
        } else {
          return it->second.Get();
        }
      }) {}

// jLatis::Latis(const LatisMsg &sheet)
// j    : metadata_(absl::make_unique<MetadataImpl>(sheet.metadata())) {
// j  for (const auto &cell : sheet.cells()) {
// j    cells_[XY::From(cell.point_location())] = CellObj(cell);
// j  }
// j}

StatusOr<Amount> Latis::Get(XY xy) {
  if (const auto it = cells_.find(xy); it != cells_.end()) {
    return it->second.Get();
  }

  return Status(INVALID_ARGUMENT, "");
}

std::string Latis::Print(XY xy) const {
  if (const auto &it = cells_.find(xy); it != cells_.end()) {
    return it->second.Print();
  }
  return "";
}

Status Latis::Set(XY xy, std::string_view input) {
  CellObj c;
  ASSIGN_OR_RETURN_(c, CellObj::From(xy, input, GetLookupFn()));
  cells_[xy] = c;
  return OkStatus();
}

LatisMsg Latis::Write() const {
  LatisMsg latis_msgb;

  metadata_->WriteTo(&latis_msgb);

  for (const auto &[pt, cell] : cells_) {
    *latis_msgb.add_cells() = cell.Export();
  }

  return latis_msgb;
}

Latis::CellObj::CellObj(XY xy, Amount amount) {
  *cell_.mutable_point_location() = xy.ToPointLocation();
  *cell_.mutable_amount() = amount;
}

Latis::CellObj::CellObj(XY xy, Expression expression) {
  *cell_.mutable_point_location() = xy.ToPointLocation();
  *cell_.mutable_formula()->mutable_expression() = expression;
  // TODO cache amount FIXME
  // *cell_.mutable_formula().set_cached_amount() = amount;
}

::google::protobuf::util::StatusOr<Latis::CellObj>
Latis::CellObj::CellObj::From(XY xy, std::string_view input,
                              formula::LookupFn *lookup_fn) {
  Amount a;
  ASSIGN_OR_RETURN_(a, formula::Parse(input, *lookup_fn));
  return Latis::CellObj(xy, a);
}

Amount Latis::CellObj::CellObj::Get() const {
  return cell_.has_amount() ? cell_.amount() : cell_.formula().cached_amount();
}

void Latis::UpdateEditTime() {
  absl::MutexLock l(&mu_);
  metadata_->UpdateEditTime();
}

formula::LookupFn *Latis::GetLookupFn() {
  static formula::LookupFn fn = [&](XY xy) -> absl::optional<Amount> {
    if (const auto it = cells_.find(xy); it == cells_.end()) {
      return absl::nullopt;
    } else {
      return it->second.Get();
    }
  };

  return &fn;
}

} // namespace latis
