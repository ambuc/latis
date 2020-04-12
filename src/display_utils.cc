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

std::string GetBorder(internal::BorderStyle style,
                      internal::BorderPiece piece) {
  switch (style) {
  case (internal::BorderStyle::kAscii): {
    static std::string sample = R"(++-+)"  // 00 01 02 03
                                R"(++-+)"  // 04 05 06 07
                                R"(||..)"  // 08 09 10 11
                                R"(++.+)"; // 12 13 14 15
    return sample.substr(int(piece), 1);
  }
  case (internal::BorderStyle::kBoxDrawing): {
    static std::string sample = R"(┌┬─┐)"  // 00 01 02 03
                                R"(├┼─┤)"  // 04 05 06 07
                                R"(││░░)"  // 08 09 10 11
                                R"(└┴░┘)"; // 12 13 14 15
    static int char_width = 3;
    return sample.substr(char_width * int(piece), char_width);
  }
  case (internal::BorderStyle::kFancyBoxDrawing): {
    static std::string sample = R"(╔╤═╗)"  // 00 01 02 03
                                R"(╟┼─╢)"  // 04 05 06 07
                                R"(║│▒▒)"  // 08 09 10 11
                                R"(╚╧▒╝)"; // 12 13 14 15
    static int char_width = 3;
    return sample.substr(char_width * int(piece), char_width);
  }
  default:
    return "?";
  }
}

std::string Pad(absl::string_view input, int n) {
  return absl::StrFormat("%*s", n, input);
}
void Empty(std::ostream &os, int n) {
  for (int i = 0; i < n; ++i) {
    os << " ";
  }
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

void GridView::HorizontalSeparatorFirst(std::ostream &os) const {
  return HorizontalSeparator(
      os, GetBorder(border_style_, internal::BorderPiece::kNWCorner),
      GetBorder(border_style_, internal::BorderPiece::kHorizontalOuter),
      GetBorder(border_style_, internal::BorderPiece::kNorthEdge),
      GetBorder(border_style_, internal::BorderPiece::kNECorner));
}
void GridView::HorizontalSeparatorMiddle(std::ostream &os) const {
  return HorizontalSeparator(
      os, GetBorder(border_style_, internal::BorderPiece::kWestEdge),
      GetBorder(border_style_, internal::BorderPiece::kHorizontalInner),
      GetBorder(border_style_, internal::BorderPiece::kCrossroads),
      GetBorder(border_style_, internal::BorderPiece::kEastEdge));
}
void GridView::HorizontalSeparatorLast(std::ostream &os) const {
  return HorizontalSeparator(
      os, GetBorder(border_style_, internal::BorderPiece::kSWCorner),
      GetBorder(border_style_, internal::BorderPiece::kHorizontalOuter),
      GetBorder(border_style_, internal::BorderPiece::kSouthEdge),
      GetBorder(border_style_, internal::BorderPiece::kSECorner));
}
void GridView::HorizontalSeparator(std::ostream &os, std::string left,
                                   std::string fill, std::string middle,
                                   std::string right) const {
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
  size_t row_col_length = std::to_string(gv.height_ - 1).length();
  if (gv.show_coordinates_) {
    Empty(os, row_col_length + 2);
    os << " "; // +
    for (size_t x = 0; x < gv.width_; ++x) {
      os << " " << Pad(XY::IntegerToColumnLetter(x), gv.widths_[x]) << "  ";
    }
    os << "\n";
  }
  for (size_t y = 0; y < gv.height_; ++y) {
    if (y == 0) {
      if (gv.show_coordinates_) {
        Empty(os, row_col_length + 2);
      }
      gv.HorizontalSeparatorFirst(os);
    }
    for (size_t x = 0; x < gv.width_; ++x) {
      if (gv.show_coordinates_ && x == 0) {
        os << " " << Pad(std::to_string(y + 1), row_col_length) << " ";
      }
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

    if (gv.show_coordinates_) {
      Empty(os, row_col_length + 2);
    }
    if (y != gv.height_ - 1) {
      gv.HorizontalSeparatorMiddle(os);
    } else {
      gv.HorizontalSeparatorLast(os);
    }
  }

  return;
}

} // namespace latis
