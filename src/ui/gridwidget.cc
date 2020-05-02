
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

#include "src/ui/gridwidget.h"

namespace latis {
namespace ui {

GridWidget::GridWidget(Dimensions dimensions, int num_lines, int num_cols)
    : Widget(absl::make_unique<Window>(dimensions, BorderStyle::kNone)), //
      height_(dimensions.nlines), width_(dimensions.ncols),              //
      num_lines_(num_lines), num_cols_(num_cols),                        //
      cell_width_(std::min(width_ / num_cols_, 15)), cell_height_(3),    //
      widgets_array_() {
  Debug(absl::StrFormat("GridWidget::GridWidget(%s,%d,%d)",
                        dimensions.ToString(), num_lines, num_cols));

  widgets_array_.resize(num_cols_);
  for (std::vector<std::shared_ptr<Widget>> &v : widgets_array_) {
    v.resize(num_lines_);
  }
}

bool GridWidget::Process(int ch, const MEVENT &event, bool is_mouse) {
  Debug(absl::StrFormat("GridWidget::Process(%c)", ch));

  // optimization for sending characters to the existing TextWidget, if there is
  // one.
  if (active_ != nullptr) {
    if (active_->Process(ch, event, is_mouse)) {
      return true;
    } else {
      active_.reset();
    }
  }

  for (std::vector<std::shared_ptr<Widget>> &v : widgets_array_) {
    for (std::shared_ptr<Widget> &cell : v) {
      if (cell != nullptr) {
        if (cell->Process(ch, event, is_mouse)) {
          active_ = cell;
          return true;
        }
      }
    }
  }

  return false;
}

} // namespace ui
} // namespace latis
