
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

namespace {
// better than a hard dep. on :xy_lib
const int kCapitalLetterA = (int)'A';

std::string IntegerToColumnLetter(int i) {
  std::string v;
  if (const int r = i / 26; r > 0) {
    v.append(IntegerToColumnLetter(r - 1));
  }
  v.push_back((char)(kCapitalLetterA + (i % 26)));
  return v;
}
} // namespace

GridWidget::GridWidget(Dimensions dimensions, int num_lines, int num_cols)
    : Widget(absl::make_unique<Window>(
          dimensions, Style{.border_style = BorderStyle::kBorderStyleNone})), //
      height_(dimensions.nlines - 2), width_(dimensions.ncols - 3),           //
      num_lines_(num_lines), num_cols_(num_cols),                             //
      cell_width_(std::min(width_ / num_cols_, 15)), cell_height_(3),         //
      coordinate_markers_(), widgets_array_() {
  Debug(absl::StrFormat("GridWidget::GridWidget(%s,%d,%d)",
                        dimensions.ToString(), num_lines, num_cols));

  // make coordinate_markers;
  coordinate_markers_.resize(num_cols_ + num_lines_);
  // Column headers
  for (int i = 0; i < num_cols_; i++) {
    auto w = absl::make_unique<TextWidget>(
        window_->GetDerwin(Dimensions{.nlines = col_header_height_,
                                      .ncols = col_header_width_,
                                      .begin_y = 0,
                                      .begin_x = ((cell_width_ - 1) * i) + 1},
                           Style{
                               .border_style = BorderStyle::kBorderStyleNone,
                               .corner_style = CornerStyle::kCornerStyleNone,
                               .xpad = 1,
                               .ypad = 0,
                               .halign = HorizontalAlignment::kCenter,
                           }));
    w->Update(IntegerToColumnLetter(i));
    coordinate_markers_.push_back(std::move(w));
  }
  //  Row headers
  for (int i = 0; i < num_lines_; i++) {
    auto w = absl::make_unique<TextWidget>(window_->GetDerwin(
        Dimensions{
            .nlines = row_header_height_,
            .ncols = row_header_width_,
            .begin_y = (cell_height_ - 1) * i + col_header_height_,
            .begin_x = 0,
        },
        Style{
            .border_style = BorderStyle::kBorderStyleNone,
            .corner_style = CornerStyle::kCornerStyleNone,
            .xpad = 1,
            .ypad = 1,
        }));
    w->Update(std::to_string(i + 1));
    coordinate_markers_.push_back(std::move(w));
  }

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
