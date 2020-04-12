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

namespace {
std::string Pad(absl::string_view input, int n) {
  return absl::StrFormat("%*s", n, input);
}
} // namespace

std::string PrintAmount(const Amount &amount, const FmtOptions &afo) {
  switch (amount.amount_demux_case()) {
  case Amount::AMOUNT_DEMUX_NOT_SET: {
    return "?";
  }
  case Amount::kStrAmount: {
    return absl::StrFormat("'%s'", amount.str_amount());
  }
  case Amount::kBoolAmount: {
    return amount.bool_amount() ? "True" : "False";
  }
  case Amount::kIntAmount: {
    return absl::StrFormat("%d", amount.int_amount());
  }
  case Amount::kDoubleAmount: {
    return absl::StrFormat("%.*f", afo.double_precision,
                           amount.double_amount());
  }
  case Amount::kTimestampAmount: {
    return absl::StrFormat("%s", absl::FormatTime(absl::FromUnixSeconds(
                                     amount.timestamp_amount().seconds())));
  }
  case Amount::kMoneyAmount: {
    switch (amount.money_amount().currency()) {
    case Money::USD: {
      return absl::StrFormat("$%d.%02d", amount.money_amount().dollars(),
                             amount.money_amount().cents());
    }
    default: {
      return "?";
    }
    }
  }
  }
  return "?";
}

std::string PrintAmount(const Amount &amount) {
  return PrintAmount(amount, FmtOptions());
}
std::string PrintCell(const Cell &cell, const FmtOptions &afo) {
  return PrintAmount(cell.formula().cached_amount(), afo);
}
std::string PrintCell(const Cell &cell) {
  return PrintCell(cell, FmtOptions());
}

void GridView::Write(XY xy, const Cell *cell_ptr) {
  size_t y = xy.Y() - offset_y_;
  size_t x = xy.X() - offset_x_;
  if (0 <= y && y < height_ && 0 <= x && x < width_) {
    strings_[XY(x, y)] = PrintCell(
        *cell_ptr, FmtOptions({.double_precision = double_precision_}));
    widths_[x] = std::max(widths_[x], strings_[XY(x, y)].length());
  }
}

void GridView::PutHline(std::ostream &os) const {
  for (size_t x = 0; x < width_; ++x) {
    if (x == 0) {
      os << '+';
    }
    // :)
    os.fill('-');
    os.width(widths_[x] + 3);
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
  for (size_t y = 0; y < gv.height_; ++y) {
    for (size_t x = 0; x < gv.width_; ++x) {
      auto xy = XY(x, y);
      if (x == 0) {
        os << "|";
      }
      os << " "
         << Pad(
                [&gv, &xy]() -> std::string {
                  if (const auto it = gv.strings_.find(xy);
                      it != gv.strings_.end()) {
                    return it->second;
                  } else {
                    return "";
                  }
                }(),
                gv.widths_[x])
         << " |";
    }
    os << "\n";
    gv.PutHline(os);
  }

  return;
}

} // namespace latis
