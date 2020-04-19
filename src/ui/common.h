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

#include "absl/strings/string_view.h"
#include <ncurses.h>

namespace latis {
namespace ui {

// general-purpose ui options.
struct Opts {
  bool show_dimensions = false;
  bool show_borders = false;
  bool show_debug_textbox = false;
};

struct Dimensions {
  int nlines;
  int ncols;
  int begin_y;
  int begin_x;

  // Returns true if the (y,x) coord is within this window.
  bool Contains(int y, int x) const {
    return (begin_y <= y && y <= (begin_y + nlines - 1)) &&
           (begin_x <= x && x <= (begin_x + ncols - 1));
  }
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_COMMON_H_