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

enum BorderStyle { kBorderStyleNone, kThin, kThick, kDouble };
enum CornerStyle { kCornerStyleNone, kClosed, kPlus };
enum HorizontalAlignment { kLeft, kCenter, kRight };
enum VerticalAlignment { kTop, kMiddle, kBottom };

// Defaults
struct Style {
  BorderStyle border_style = BorderStyle::kThin;
  CornerStyle corner_style = CornerStyle::kClosed;
  int xpad = 1;
  int ypad = 0;
  HorizontalAlignment halign = HorizontalAlignment::kLeft;
  VerticalAlignment valign = VerticalAlignment::kMiddle;
};

struct Dimensions {
  int nlines;
  int ncols;
  int begin_y;
  int begin_x;

  bool operator==(Dimensions other) const;

  std::string ToString() const;

  int Width() const { return ncols; }
  int Height() const { return nlines; }

  int LeftEdge() const { return begin_x; }
  int RightEdge() const { return LeftEdge() + Width(); }
  int TopEdge() const { return begin_y; }
  int BottomEdge() const { return TopEdge() + Height(); }

  bool CollidesWith(const Dimensions &other) const;
};

void Debug(absl::string_view s);

} // namespace ui
} // namespace latis

#endif // SRC_UI_COMMON_H_
