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
#ifndef SRC_UI_LAYOUT_ENGINE_H_
#define SRC_UI_LAYOUT_ENGINE_H_

#include "src/ui/common.h"

#include "absl/types/optional.h"

namespace latis {
namespace ui {

class LayoutEngine {
public:
  LayoutEngine(int height, int width) : height_(height), width_(width) {}

  // Places a box as far in the top-left as it can go.
  absl::optional<Dimensions> PlaceTL(int h, int w);

private:
  // Regardless of boxes, true/false is within borders.
  bool InBorders(int h, int w, int begin_y, int begin_x);

  const int height_;
  const int width_;

  std::vector<Dimensions> boxes_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_LAYOUT_ENGINE_H_
