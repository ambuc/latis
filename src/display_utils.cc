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

std::string GetAsciiBorder(internal::BorderPiece piece) {
  switch (piece) {
  case (internal::BorderPiece::kVerticalInner):
    return "|";
  case (internal::BorderPiece::kVerticalOuter):
    return "|";
  case (internal::BorderPiece::kHorizontalInner):
    return "-";
  case (internal::BorderPiece::kHorizontalOuter):
    return "-";
  case (internal::BorderPiece::kNorthEdge):
    return "+";
  case (internal::BorderPiece::kEastEdge):
    return "+";
  case (internal::BorderPiece::kSouthEdge):
    return "+";
  case (internal::BorderPiece::kWestEdge):
    return "+";
  case (internal::BorderPiece::kNWCorner):
    return "+";
  case (internal::BorderPiece::kNECorner):
    return "+";
  case (internal::BorderPiece::kSWCorner):
    return "+";
  case (internal::BorderPiece::kSECorner):
    return "+";
  case (internal::BorderPiece::kCrossroads):
    return "+";
  default:
    return "?";
  }
};
std::string GetBoxDrawingBorder(internal::BorderPiece piece) {
  switch (piece) {
  case (internal::BorderPiece::kVerticalInner):
    return "│";
  case (internal::BorderPiece::kVerticalOuter):
    return "│";
  case (internal::BorderPiece::kHorizontalInner):
    return "─";
  case (internal::BorderPiece::kHorizontalOuter):
    return "─";
  case (internal::BorderPiece::kNorthEdge):
    return "┬";
  case (internal::BorderPiece::kEastEdge):
    return "┤";
  case (internal::BorderPiece::kSouthEdge):
    return "┴";
  case (internal::BorderPiece::kWestEdge):
    return "├";
  case (internal::BorderPiece::kNWCorner):
    return "┌";
  case (internal::BorderPiece::kNECorner):
    return "┐";
  case (internal::BorderPiece::kSWCorner):
    return "└";
  case (internal::BorderPiece::kSECorner):
    return "┘";
  case (internal::BorderPiece::kCrossroads):
    return "┼";
  default:
    return "?";
  }
};
std::string GetFancyBoxDrawingBorder(internal::BorderPiece piece) {
  switch (piece) {
  case (internal::BorderPiece::kVerticalInner):
    return "│";
  case (internal::BorderPiece::kVerticalOuter):
    return "║";
  case (internal::BorderPiece::kHorizontalInner):
    return "─";
  case (internal::BorderPiece::kHorizontalOuter):
    return "═";
  case (internal::BorderPiece::kNorthEdge):
    return "╤";
  case (internal::BorderPiece::kEastEdge):
    return "╢";
  case (internal::BorderPiece::kSouthEdge):
    return "╧";
  case (internal::BorderPiece::kWestEdge):
    return "╟";
  case (internal::BorderPiece::kNWCorner):
    return "╔";
  case (internal::BorderPiece::kNECorner):
    return "╗";
  case (internal::BorderPiece::kSWCorner):
    return "╚";
  case (internal::BorderPiece::kSECorner):
    return "╝";
  case (internal::BorderPiece::kCrossroads):
    return "┼";
  default:
    return "?";
  }
};

std::string GetBorder(internal::BorderStyle style,
                      internal::BorderPiece piece) {
  switch (style) {
  case (internal::BorderStyle::kAscii):
    return GetAsciiBorder(piece);
  case (internal::BorderStyle::kBoxDrawing):
    return GetBoxDrawingBorder(piece);
  case (internal::BorderStyle::kFancyBoxDrawing):
    return GetFancyBoxDrawingBorder(piece);
  default:
    return " ";
  }
}

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
  const auto local_xy = XY(x, y);
  if (0 <= y && y < height_ && 0 <= x && x < width_) {
    strings_[local_xy] = PrintCell(*cell_ptr, fmt_options_);
    widths_[x] = std::max(widths_[x], strings_[local_xy].length());
  }
}

void GridView::PutTopHline(std::ostream &os) const {
  return PutHline(
      os, GetBorder(border_style_, internal::BorderPiece::kNWCorner),
      GetBorder(border_style_, internal::BorderPiece::kHorizontalOuter),
      GetBorder(border_style_, internal::BorderPiece::kNorthEdge),
      GetBorder(border_style_, internal::BorderPiece::kNECorner));
}
void GridView::PutMiddleHline(std::ostream &os) const {
  return PutHline(
      os, GetBorder(border_style_, internal::BorderPiece::kWestEdge),
      GetBorder(border_style_, internal::BorderPiece::kHorizontalInner),
      GetBorder(border_style_, internal::BorderPiece::kCrossroads),
      GetBorder(border_style_, internal::BorderPiece::kEastEdge));
}
void GridView::PutBottomHline(std::ostream &os) const {
  return PutHline(
      os, GetBorder(border_style_, internal::BorderPiece::kSWCorner),
      GetBorder(border_style_, internal::BorderPiece::kHorizontalOuter),
      GetBorder(border_style_, internal::BorderPiece::kSouthEdge),
      GetBorder(border_style_, internal::BorderPiece::kSECorner));
}
void GridView::PutHline(std::ostream &os, std::string left, std::string fill,
                        std::string middle, std::string right) const {
  for (size_t x = 0; x < width_; ++x) {
    if (x == 0) {
      os << left;
    }
    for (size_t i = 0; i < widths_[x] + 2; ++i) {
      os << fill;
    }
    if (x != width_ - 1) {
      os << middle;
    } else {
      os << right;
    }
  }
  os << "\n";
}

void operator<<(std::ostream &os, const GridView &gv) {
  for (size_t y = 0; y < gv.height_; ++y) {
    if (y == 0) {
      gv.PutTopHline(os);
    }
    for (size_t x = 0; x < gv.width_; ++x) {
      const auto xy = XY(x, y);
      if (x == 0) {
        os << GetBorder(gv.border_style_,
                        internal::BorderPiece::kVerticalOuter);
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
         << " ";
      if (x == gv.width_ - 1) {
        os << GetBorder(gv.border_style_,
                        internal::BorderPiece::kVerticalOuter);
      } else {
        os << GetBorder(gv.border_style_,
                        internal::BorderPiece::kVerticalInner);
      }
    }
    os << "\n";

    if (y != gv.height_ - 1) {
      gv.PutMiddleHline(os);
    } else {
      gv.PutBottomHline(os);
    }
  }

  return;
}

} // namespace latis
