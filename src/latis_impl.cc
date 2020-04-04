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
#include "src/formula/evaluator.h"

#include "absl/memory/memory.h"

namespace latis {

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;

Latis::Latis() : Latis(LatisMsg()) {}

Latis::Latis(const LatisMsg &sheet)
    : title_(sheet.metadata().has_title()
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
    cells_[XY::From(cell.point_location())] = cell;
  }
}

StatusOr<Amount> Latis::Get(XY xy) {
  if (const auto it = cells_.find(xy); it != cells_.end()) {
    return it->second.formula().cached_amount();
  }

  return Status(INVALID_ARGUMENT, "");
}

Status Latis::Set(XY xy, std::string_view input) {
  // Evaluate and store lookups.
  std::vector<XY> looked_up{};

  LookupFn lookup_fn = [&](XY xy) -> absl::optional<Amount> {
    if (const auto maybe = Get(xy); maybe.ok()) {
      looked_up.push_back(xy);
      return maybe.ValueOrDie();
    }
    return absl::nullopt;
  };

  // Compute amount.
  std::tuple<Expression, Amount> ea;
  ASSIGN_OR_RETURN_(ea, formula::Parse(input, lookup_fn));

  // Insert edges.
  // TODO must invalidate. FIXME
  for (const XY &dependency : looked_up) {
    dependents_.insert({dependency, xy});
    dependencies_.insert({xy, dependency});
  }

  // Construct cell.
  Cell c;
  *c.mutable_point_location() = xy.ToPointLocation();
  *c.mutable_formula()->mutable_expression() = std::get<0>(ea);
  *c.mutable_formula()->mutable_cached_amount() = std::get<1>(ea);

  // Insert cell.
  UpdateEditTime();
  cells_[xy] = c;

  // Update dependents.
  UpdateDependents(xy);

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
    *latis_msg->add_cells() = cell;
  }

  return OkStatus();
}

void Latis::UpdateDependents(XY xy) {
  auto ret = dependents_.equal_range(xy);
  for (std::multimap<XY, XY>::iterator it = ret.first; it != ret.second; ++it) {
    Update(it->second);
  }
}

void Latis::Update(XY xy) {
  LookupFn lookup_fn = [&](XY xy) -> absl::optional<Amount> {
    if (const auto maybe = Get(xy); maybe.ok()) {
      return maybe.ValueOrDie();
    }
    return absl::nullopt;
  };

  if (const StatusOr<Amount> amt =
          formula::Evaluator(lookup_fn).CrunchExpression(
              cells_[xy].formula().expression());
      amt.ok()) {
    *cells_[xy].mutable_formula()->mutable_cached_amount() = amt.ValueOrDie();
  }
}

void Latis::UpdateEditTime() {
  absl::MutexLock l(&mu_);
  edited_time_ = absl::Now();
}

} // namespace latis
