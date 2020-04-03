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

Latis::Latis() {}

Latis::Latis(const LatisMsg &sheet)
    : metadata_(absl::make_unique<MetadataImpl>(sheet.metadata())) {
  for (const auto &cell : sheet.cells()) {
    cells_[XY::From(cell.point_location())] = CellObj(cell);
  }
}

StatusOr<Amount> Latis::Get(XY xy) {
  if (const auto it = cells_.find(xy); it != cells_.end()) {
    return it->second.GetAmount();
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
  for (const auto [xy, cobj] : cells_) {
    std::cout << xy.ToA1() << ", " << cobj.Print() << std::endl;
  }
  return OkStatus();
}

LatisMsg Latis::Write() const {
  LatisMsg latis_msgb;

  metadata_->WriteTo(&latis_msgb);

  // cells
  for (const auto &[pt, cell] : cells_) {
    *latis_msgb.add_cells() = cell.Export();
  }

  return latis_msgb;
}

formula::LookupFn *Latis::GetLookupFn() {
  static formula::LookupFn fn = [&](XY xy) -> absl::optional<Amount> {
    auto it = cells_.find(xy);
    if (it == cells_.end()) {
      return absl::nullopt;
    }
    return it->second.GetAmount();
  };

  return &fn;
}

} // namespace latis
