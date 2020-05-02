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
#ifndef SRC_UI_GRIDWIDGET_H_
#define SRC_UI_GRIDWIDGET_H_

#include "src/ui/widget.h"

#include "absl/memory/memory.h"

namespace latis {
namespace ui {

class GridWidget : public Widget {
public:
  GridWidget(Dimensions dimensions, int num_lines, int num_cols);
  ~GridWidget() override {}

  template <typename T, typename... Args> //
  T *Add(int y, int x, Args... args) {
    Debug(absl::StrFormat("GridWidget::Add(%d, %d)", y, x));
    widgets_array_[y][x] =
        std::make_shared<T>(args..., window_->GetDerwin(
                                         /*dimensions=*/
                                         Dimensions{
                                             .nlines = cell_height_,
                                             .ncols = cell_width_,
                                             .begin_y = (cell_height_ * y) - y,
                                             .begin_x = (cell_width_ * x) - x,
                                         }));
    return Get<T>(y, x);
  }

  template <typename T> //
  T *Get(int y, int x) {
    Debug(absl::StrFormat("GridWidget::Get(%d, %d)", y, x));
    return static_cast<T *>(widgets_array_[y][x].get());
  }

  // Returns true if this widget consumed the event.
  bool Process(int ch, const MEVENT &event, bool is_mouse) override;

private:
  const int height_;    // Height of the grid.
  const int width_;     // Width of the grid.
  const int num_lines_; // Number of rows in the grid.
  const int num_cols_;  // Number of columns in the grid.
  const int cell_width_;
  const int cell_height_;

  // if nullptr, none is selected.
  std::shared_ptr<Widget> active_;

  std::vector<std::vector<std::shared_ptr<Widget>>> widgets_array_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_GRIDWIDGET_H_
