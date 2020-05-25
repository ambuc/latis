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

#include "absl/container/flat_hash_map.h"
#include "src/ui/textwidget.h"

namespace latis {
namespace ui {

class GridWidget : public Widget {
public:
  GridWidget(Dimensions dimensions);
  ~GridWidget() override {}

  // returns nullptr if there is no room.
  template <typename T, typename... Args> //
  std::shared_ptr<T> AddCell(int y, int x, Args... args) {
    Debug(absl::StrFormat("GridWidget::AddCell(%d, %d)", y, x));
    auto p = std::make_shared<T>(
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
    widgets_[std::make_pair(y, x)] = p;
    // if created recently, set active.
    Debug(absl::StrFormat("Setting %d, %d to active.", y, x));
    active_ = std::make_unique<ActiveWidget>(p, y, x);
    return p;
  }

  template <typename T> //
  std::shared_ptr<T> Get(int y, int x) {
    Debug(absl::StrFormat("GridWidget::Get(%d, %d)", y, x));
    const auto pair = std::make_pair(y, x);
    const auto it = widgets_.find(pair);
    if (it == widgets_.end()) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<T>(it->second);
  }

  // Returns true if successful.
  bool SetActive(int y, int x) {
    const auto it = widgets_.find(std::make_pair(y, x));
    if (it == widgets_.end()) {
      return false;
    }
    active_ = std::make_unique<ActiveWidget>(it->second, y, x);
    return true;
  }

  // Returns true if this widget consumed the event.
  bool Process(int ch) override;

  void Focus() override {}
  void UnFocus() override {}

private:
  const int height_; // Height of the grid.
  const int width_;  // Width of the grid.
  const int cell_width_;
  const int cell_height_;

  // for headers
  const int col_header_height_{1};
  const int row_header_width_{3};

  std::unique_ptr<ActiveWidget> active_;

  std::vector<std::shared_ptr<TextWidget>> coordinate_markers_;
  absl::flat_hash_map<std::pair<int, int>, std::shared_ptr<Widget>> widgets_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_GRIDWIDGET_H_
