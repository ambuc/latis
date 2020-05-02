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

#include "src/ui/layout_engine.h"

#include <iostream>

namespace latis {
namespace ui {

absl::optional<Dimensions> LayoutEngine::Place(int h, int w) {
  auto d = Dimensions{.nlines = h, .ncols = w, .begin_y = 0, .begin_x = 0};

  while (true) {
    auto d_lcl = d;
    for (const auto &box : boxes_) {
      if (box.CollidesWith(d)) {
        if (box.RightEdge() + w <= width_) {
          d.begin_x = box.RightEdge();
        } else {
          // newline
          d.begin_y++;
          d.begin_x = 0;
        }
      }
    }
    if (d_lcl == d) {
      break;
    }
  }

  if (!InBorders(h, w, d.begin_y, d.begin_x)) {
    return absl::nullopt;
  }

  boxes_.push_back(d);

  return d;
}

absl::optional<Dimensions> LayoutEngine::FillRest() {
  // find the bottommost box;
  int top_edge = 0;
  for (const auto &box : boxes_) {
    top_edge = std::max(top_edge, box.BottomEdge());
  }
  if (height_ == top_edge) {
    return absl::nullopt;
  }
  return Dimensions{.nlines = height_ - top_edge,
                    .ncols = width_,
                    .begin_y = top_edge,
                    .begin_x = 0};
}

bool LayoutEngine::InBorders(int h, int w, int begin_y, int begin_x) {
  return begin_x + w <= width_ && begin_y + h <= height_;
}

} // namespace ui
} // namespace latis
