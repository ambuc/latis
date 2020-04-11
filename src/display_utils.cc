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

std::string PrintAmount(const Amount &amount, const AmountFormatOptions &afo) {
  switch (amount.amount_demux_case()) {
  case Amount::AMOUNT_DEMUX_NOT_SET: {
    return "?";
  }
  case Amount::kStrAmount: {
    return amount.str_amount();
  }
  case Amount::kBoolAmount: {
    return amount.bool_amount() ? "True" : "False";
  }
  case Amount::kIntAmount: {
    return absl::StrFormat(afo.int_format_options, amount.int_amount());
  }
  case Amount::kDoubleAmount: {
    return absl::StrFormat(afo.double_format_options, amount.double_amount());
  }
  case Amount::kTimestampAmount: {
    return absl::FormatTime(
        absl::FromUnixSeconds(amount.timestamp_amount().seconds()));
  }
  case Amount::kMoneyAmount: {
    switch (amount.money_amount().currency()) {
    case Money::USD: {
      return absl::StrFormat("$%d.%02d", amount.money_amount().dollars(),
                             amount.money_amount().cents());
    }
    default: {
      return "?$";
    }
    }
  }
  }
  return "?";
}

std::string PrintCell(const Cell &cell, const AmountFormatOptions &afo) {
  return PrintAmount(cell.formula().cached_amount(), afo);
}

void GridView::Write(XY xy, const Cell *cell_ptr) {
  int y = xy.Y() - offset_y_;
  int x = xy.X() - offset_x_;
  cells_[y][x] = cell_ptr;
  strings_[y][x] = PrintCell(*cell_ptr);
  widths_[x] = std::max(widths_[x], int(strings_[y][x].value_or("").length()));
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

std::ostream &operator<<(std::ostream &os, const GridView &gv) {
  if (gv.height_ == 0 && gv.width_ == 0) {
    return os;
  }

  gv.PutHline(os);

  // For each row:
  for (int y = 0; y < gv.strings_.size(); ++y) {
    for (int x = 0; x < gv.strings_[y].size(); ++x) {
      if (x == 0) {
        os << "|";
      }
      os << " " << gv.strings_[y][x].value_or("") << " |";
    }
    os << "\n";
    gv.PutHline(os);
  }

  return os;
}

} // namespace latis
