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

#include "src/display_utils.h"

#include "absl/strings/str_format.h"
#include "absl/time/time.h"

namespace latis {

std::string PrintAmount(const Amount &amount, const FmtOptions &afo) {
  switch (amount.amount_demux_case()) {
  case Amount::AMOUNT_DEMUX_NOT_SET: {
    return absl::StrFormat("%*s", afo.width, "?");
  }
  case Amount::kStrAmount: {
    return absl::StrFormat("%*s", afo.width,
                           absl::StrFormat("'%s'", amount.str_amount()));
  }
  case Amount::kBoolAmount: {
    return absl::StrFormat("%*s", afo.width,
                           amount.bool_amount() ? "True" : "False");
  }
  case Amount::kIntAmount: {
    return absl::StrFormat("%*d", afo.width, amount.int_amount());
  }
  case Amount::kDoubleAmount: {
    return absl::StrFormat("%*.*f", afo.width, afo.double_precision,
                           amount.double_amount());
  }
  case Amount::kTimestampAmount: {
    return absl::StrFormat("%*s", afo.width,
                           absl::FormatTime(absl::FromUnixSeconds(
                               amount.timestamp_amount().seconds())));
  }
  case Amount::kMoneyAmount: {
    switch (amount.money_amount().currency()) {
    case Money::USD: {
      return absl::StrFormat("%*s", afo.width,
                             absl::StrFormat("$%d.%02d",
                                             amount.money_amount().dollars(),
                                             amount.money_amount().cents()));
    }
    default: {
      return absl::StrFormat("%*s", afo.width, "?");
    }
    }
  }
  }
  return absl::StrFormat("%*s", afo.width, "?");
}

std::string PrintCell(const Cell &cell, const FmtOptions &afo) {
  return PrintAmount(cell.formula().cached_amount(), afo);
}

void GridView::Write(XY xy, const Cell *cell_ptr) {
  int y = xy.Y() - offset_y_;
  int x = xy.X() - offset_x_;
  if (0 <= y && y < height_ && 0 <= x && x < width_) {
    cells_[y][x] = cell_ptr;
    // TODO(ambuc): We PrintCell() 2x per cell. There's an optimization here
    // where we store the unpadded strings and then pad them at print time.
    int w = PrintCell(*cell_ptr, FmtOptions({
                                     .width = -1,
                                     .double_precision = double_precision_,
                                 }))
                .length();
    widths_[x] = std::max(widths_[x], w);
  }
}

void GridView::PutHline(std::ostream &os) const {
  for (int x = 0; x < width_; ++x) {
    if (x == 0) {
      os << '+';
    }
    for (int i = 0; i < widths_[x] + 2; ++i) {
      os << "-";
    }
    os << '+';
  }
  os << "\n";
}

void operator<<(std::ostream &os, const GridView &gv) {
  if (gv.height_ == 0 && gv.width_ == 0) {
    return;
  }

  gv.PutHline(os);

  // For each row:
  for (size_t y = 0; y < gv.cells_.size(); ++y) {
    for (size_t x = 0; x < gv.cells_[y].size(); ++x) {
      if (x == 0) {
        os << "|";
      }
      if (gv.cells_[y][x] == nullptr) {
        for (int i = 0; i < gv.widths_[x] + 2; ++i) {
          os << " ";
        }
        os << "|";
      } else {
        os << " "
           << PrintCell(*gv.cells_[y][x],
                        {.width = gv.widths_[x],
                         .double_precision = gv.double_precision_})
           << " |";
      }
    }
    os << "\n";
    gv.PutHline(os);
  }

  return;
}

} // namespace latis
