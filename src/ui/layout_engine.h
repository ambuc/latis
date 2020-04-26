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

  absl::optional<Dimensions> Place(int y, int x);

  // Returns true if successful.
  bool Newline();

  // void Reflow(); easy enough to write, how to communicate the new positions
  // upstream?

private:
  const int height_;
  const int width_;

  int next_y_{0};
  int next_x_{0};
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_LAYOUT_ENGINE_H_
