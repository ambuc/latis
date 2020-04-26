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

namespace latis {
namespace ui {

absl::optional<Dimensions> LayoutEngine::Place(int y, int x) {

  return Dimensions{.nlines = y, .ncols = x, .begin_y = 0, .begin_x = 0};
}
bool LayoutEngine::Newline() {
  if (next_y_ < height_ - 2) {
    next_y_++;
    next_x_ = 0;
    // TODO pick up from here
  }
}

} // namespace ui
} // namespace latis
