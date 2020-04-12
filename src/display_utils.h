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

#ifndef SRC_DISPLAY_UTILS_H_
#define SRC_DISPLAY_UTILS_H_

#include "proto/latis_msg.pb.h"
#include "src/xy.h"

#include <iostream>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"

namespace latis {

namespace internal {

// ┌┬─┐ // 00 01 02 03
// ├┼─┤ // 04 05 06 07
// ││.. // 08 09
// └┴.┘ // 12 13    15
enum BorderPiece {
  kNWCorner = 0,
  kNorthEdge = 1,
  kHorizontalOuter = 2,
  kNECorner = 3,
  kWestEdge = 4,
  kCrossroads = 5,
  kHorizontalInner = 6,
  kEastEdge = 7,
  kVerticalOuter = 8,
  kVerticalInner = 9,
  kNULL00 = 10,
  kNULL01 = 11,
  kSWCorner = 12,
  kSouthEdge = 13,
  kNULL02 = 14,
  kSECorner = 15,
};

enum BorderStyle { kAbsent, kAscii, kBoxDrawing, kFancyBoxDrawing };

} // namespace internal

struct FmtOptions {
  int width = 5;
  int double_precision = 3;
};

std::string PrintCell(const Cell &cell, const FmtOptions &afo);
std::string PrintCell(const Cell &cell);
std::string PrintAmount(const Amount &amount, const FmtOptions &afo);
std::string PrintAmount(const Amount &amount);

// Short-lived class to assist with stdout grid printing. Must not outlive its
// pointees.
//
// Usage:
//   auto gv = GridView({.height=10,.width=10});
//   gv.Write(xy, &cell); ...
//   std::cout << gv << std::endl;
//
class GridView {
public:
  struct Options {
    int height = 0;
    int width = 0;

    int offset_x = 0;
    int offset_y = 0;

    // Formatting
    int double_precision = 2;

    internal::BorderStyle border_style = internal::kAscii;

    bool show_coordinates = true;
  };

  explicit GridView(Options options)
      : height_(options.height), width_(options.width),
        offset_x_(options.offset_x), offset_y_(options.offset_y),
        fmt_options_({.double_precision = options.double_precision}),
        border_style_(options.border_style),
        show_coordinates_(options.show_coordinates), widths_(width_, 0) {}

  void Write(XY xy, const Cell *cell_ptr);

  friend void operator<<(std::ostream &os, const GridView &gv);

private:
  void HorizontalSeparator(std::ostream &os, std::string left, std::string fill,
                           std::string middle, std::string right) const;
  void HorizontalSeparatorFirst(std::ostream &os) const;
  void HorizontalSeparatorMiddle(std::ostream &os) const;
  void HorizontalSeparatorLast(std::ostream &os) const;

  const size_t height_;
  const size_t width_;
  const int offset_x_;
  const int offset_y_;
  const FmtOptions fmt_options_;
  const internal::BorderStyle border_style_;
  const bool show_coordinates_;

  absl::flat_hash_map<XY, std::string> strings_;
  std::vector<size_t> widths_;
};

void operator<<(std::ostream &os, const GridView &gv);

} // namespace latis

#endif // SRC_DISPLAY_UTILS_H_
