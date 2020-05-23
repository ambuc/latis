
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

GridWidget::GridWidget(Dimensions dimensions)
    : Widget(absl::make_unique<Window>(
          dimensions, Style{.border_style = BorderStyle::kBorderStyleNone})), //
      height_(dimensions.nlines - 2), width_(dimensions.ncols - 3),           //
      cell_width_(15), cell_height_(3),                                       //
      coordinate_markers_(), widgets_array_() {
  // Debug(absl::StrFormat("GridWidget::GridWidget(%s)",
  // dimensions.ToString()));

  // Column headers
  for (int i = 0; i < width_ / (cell_width_ - 1); i++) {
    auto w = absl::make_unique<TextWidget>(
        window_->GetDerwin(Dimensions{.nlines = col_header_height_,
                                      .ncols = cell_width_,
                                      .begin_y = 0,
                                      .begin_x = ((cell_width_ - 1) * i) + 1},
                           Style{
                               .border_style = BorderStyle::kBorderStyleNone,
                               .corner_style = CornerStyle::kCornerStyleNone,
                               .xpad = 4,
                               .ypad = 0,
                               .halign = HorizontalAlignment::kLeft,
                               .color = Color::RED,
                           }));
    w->UpdateUnderlyingContent(IntegerToColumnLetter(i));
    coordinate_markers_.push_back(std::move(w));
  }

  //  Row headers
  for (int i = 0; i < height_ / (cell_height_ - 1); i++) {
    auto w = absl::make_unique<TextWidget>(window_->GetDerwin(
        Dimensions{
            .nlines = cell_height_,
            .ncols = row_header_width_,
            .begin_y = (cell_height_ - 1) * i + col_header_height_,
            .begin_x = 0,
        },
        Style{
            .border_style = BorderStyle::kBorderStyleNone,
            .corner_style = CornerStyle::kCornerStyleNone,
            .xpad = 1,
            .ypad = 1,
            .color = Color::RED,
        }));
    w->UpdateUnderlyingContent(std::to_string(i + 1));
    coordinate_markers_.push_back(std::move(w));
  }

  widgets_array_.resize(width_ / cell_width_);
  for (std::vector<std::unique_ptr<Widget>> &v : widgets_array_) {
    v.resize(height_ / cell_height_);
  }
}

int GridWidget::Process(int ch) {
  assert(active_ != nullptr);
  if (int retch = active_->Process(ch); retch == 0) {
    return 0;
  } else {
    assert(retch == KEY_LEFT || retch == KEY_RIGHT || retch == KEY_UP ||
           retch == KEY_DOWN || retch == int('q'));
    if (retch == int('q')) {
      return 0;
    }
  }

  // Debug(absl::StrFormat("GridWidget::Process(%c)", ch));

  // for (std::vector<std::unique_ptr<Widget>> &v : widgets_array_) {
  //   for (std::unique_ptr<Widget> &cell : v) {
  //     if (cell != nullptr) {
  //       if (cell->Process(ch)) {
  //         active_ = cell.get();
  //         return true;
  //       }
  //     }
  //   }
  // }

  return 0;
}

} // namespace ui
} // namespace latis
