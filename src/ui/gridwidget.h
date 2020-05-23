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

#include "src/ui/textwidget.h"

namespace latis {
namespace ui {

class GridWidget : public Widget {
public:
  GridWidget(Dimensions dimensions);
  ~GridWidget() override {}

  // returns nullptr if there is no room.
  template <typename T, typename... Args> //
  T *Add(int y, int x, Args... args) {
    Debug(absl::StrFormat("GridWidget::Add(%d, %d)", y, x));
    if (int(widgets_array_.size()) <= y || int(widgets_array_[y].size()) <= x) {
      return nullptr;
    }
    widgets_array_[y][x] = std::make_unique<T>(
        args..., window_->GetDerwin(
                     /*dimensions=*/
                     Dimensions{
                         .nlines = cell_height_,
                         .ncols = cell_width_,
                         .begin_y = (cell_height_ * y) - y + col_header_height_,
                         .begin_x = (cell_width_ * x) - x + row_header_width_,
                     },
                     Style{
                         .border_style = BorderStyle::kThin,
                         .corner_style = CornerStyle::kPlus,
                     }));
    return Get<T>(y, x);
  }

  template <typename T> //
  T *Get(int y, int x) {
    Debug(absl::StrFormat("GridWidget::Get(%d, %d)", y, x));
    return static_cast<T *>(widgets_array_[y][x].get());
  }

  // Returns true if this widget consumed the event.
  bool Process(int ch) override;

private:
  const int height_; // Height of the grid.
  const int width_;  // Width of the grid.
  const int cell_width_;
  const int cell_height_;

  // for headers
  const int col_header_height_{1};
  const int row_header_width_{3};

  // if nullptr, none is selected.
  Widget *focused_;

  std::vector<std::unique_ptr<TextWidget>> coordinate_markers_;
  std::vector<std::vector<std::unique_ptr<Widget>>> widgets_array_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_GRIDWIDGET_H_
