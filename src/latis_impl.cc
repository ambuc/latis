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

#include "absl/memory/memory.h"

namespace latis {

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;

Latis::Latis() : Latis(LatisMsg()) {}

Latis::Latis(const LatisMsg &sheet)
    : lookup_fn_([&](XY xy) -> absl::optional<Amount> {
        if (const auto it = cells_.find(xy); it == cells_.end()) {
          return absl::nullopt;
        } else {
          return it->second.Get();
        }
      }),
      title_(sheet.metadata().has_title()
                 ? absl::optional<std::string>(sheet.metadata().title())
                 : std::nullopt),
      author_(sheet.metadata().has_author()
                  ? absl::optional<std::string>(sheet.metadata().author())
                  : std::nullopt),
      created_time_(
          sheet.metadata().has_created_time()
              ? absl::FromUnixSeconds(sheet.metadata().created_time().seconds())
              : absl::Now()),
      edited_time_(
          sheet.metadata().has_edited_time()
              ? absl::FromUnixSeconds(sheet.metadata().edited_time().seconds())
              : absl::Now()) {
  for (const auto &cell : sheet.cells()) {
    cells_[XY::From(cell.point_location())] = CellObj(cell);
  }
}

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
  ASSIGN_OR_RETURN_(c, CellObj::From(xy, input, lookup_fn_));
  cells_[xy] = c;
  return OkStatus();
}

Status Latis::WriteTo(LatisMsg *latis_msg) const {
  if (title_.has_value()) {
    latis_msg->mutable_metadata()->set_title(title_.value());
  }
  if (author_.has_value()) {
    latis_msg->mutable_metadata()->set_author(author_.value());
  }
  latis_msg->mutable_metadata()->mutable_created_time()->set_seconds(
      absl::ToUnixSeconds(created_time_));
  latis_msg->mutable_metadata()->mutable_edited_time()->set_seconds(
      absl::ToUnixSeconds(edited_time_));

  for (const auto &[pt, cell] : cells_) {
    *latis_msg->add_cells() = cell.Export();
  }

  return OkStatus();
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
                              const formula::LookupFn &lookup_fn) {
  Amount a;
  ASSIGN_OR_RETURN_(a, formula::Parse(input, lookup_fn));
  return Latis::CellObj(xy, a);
}

Amount Latis::CellObj::CellObj::Get() const {
  return cell_.has_amount() ? cell_.amount() : cell_.formula().cached_amount();
}

void Latis::UpdateEditTime() {
  absl::MutexLock l(&mu_);
  edited_time_ = absl::Now();
}

} // namespace latis
