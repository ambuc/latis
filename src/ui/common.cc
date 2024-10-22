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

#include "src/ui/common.h"

#include <ncurses.h>

ABSL_FLAG(bool, debug_mode, false,
          "If true, prints debug stuff at the bottom.");

namespace latis {
namespace ui {

bool Dimensions::operator==(Dimensions other) const {
  return nlines == other.nlines && ncols == other.ncols &&
         begin_y == other.begin_y && begin_x == other.begin_x;
}

std::string Dimensions::ToString() const {
  return absl::StrFormat("%dx%d @ (%d,%d)", nlines, ncols, begin_y, begin_x);
}

bool Dimensions::CollidesWith(const Dimensions &other) const {
  if (LeftEdge() < other.LeftEdge() + other.Width() &&
      LeftEdge() + Width() > other.LeftEdge() &&
      TopEdge() < other.TopEdge() + other.Height() &&
      TopEdge() + Height() > other.TopEdge()) {
    return true;
  }
  return false;
}

void Debug(absl::string_view s) {
  if (absl::GetFlag(FLAGS_debug_mode)) {
    std::cerr << absl::GetCurrentTimeNanos() << "\t" << s << std::endl;
  }
}

} // namespace ui
} // namespace latis
