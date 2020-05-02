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
#ifndef SRC_UI_COMMON_H_
#define SRC_UI_COMMON_H_

#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

#include "absl/flags/flag.h"

#include <iostream>

ABSL_DECLARE_FLAG(bool, debug_mode);

namespace latis {
namespace ui {

enum BorderStyle { kNone, kThin, kThick, kDouble };

struct Style {
  BorderStyle border_style = BorderStyle::kNone;
};

struct Dimensions {
  int nlines;
  int ncols;
  int begin_y;
  int begin_x;

  inline bool operator==(Dimensions other) const {
    return nlines == other.nlines && ncols == other.ncols &&
           begin_y == other.begin_y && begin_x == other.begin_x;
  }

  std::string ToString() const {
    return absl::StrFormat("%dx%d @ (%d,%d)", nlines, ncols, begin_y, begin_x);
  }

  int Width() const { return ncols; }
  int Height() const { return nlines; }

  int LeftEdge() const { return begin_x; }
  int RightEdge() const { return LeftEdge() + Width(); }
  int TopEdge() const { return begin_y; }
  int BottomEdge() const { return TopEdge() + Height(); }

  bool CollidesWith(const Dimensions &other) const {
    if (LeftEdge() < other.LeftEdge() + other.Width() &&
        LeftEdge() + Width() > other.LeftEdge() &&
        TopEdge() < other.TopEdge() + other.Height() &&
        TopEdge() + Height() > other.TopEdge()) {
      return true;
    }
    return false;
  }
};

void Debug(absl::string_view s);

} // namespace ui
} // namespace latis

#endif // SRC_UI_COMMON_H_
