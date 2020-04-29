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
#ifndef SRC_UI_WIDGET_H_
#define SRC_UI_WIDGET_H_

#include "src/ui/common.h"
#include "src/ui/window.h"

#include "absl/memory/memory.h"
#include "absl/types/optional.h"
#include <form.h>

namespace latis {
namespace ui {

class Widget {
public:
  Widget(std::unique_ptr<Window> window);
  virtual ~Widget() = default;

  void Clear();

  // Returns true if this widget consumed the event.
  virtual bool Process() = 0;

protected:
  void Debug(absl::string_view txt);

  const std::unique_ptr<Window> window_;
};

// Forms are hard.
// https://invisible-island.net/ncurses/ncurses-intro.html#form
class FormWidget : public Widget {
public:
  FormWidget(std::unique_ptr<Window> window, absl::string_view placeholder);
  ~FormWidget() override;

  // Returns true if this widget consumed the event.
  bool Process() override;

  std::string Extract();

private:
  FORM *form_{nullptr};
  // Inherently just a single field.
  FIELD *fields_[2]{nullptr, nullptr};
};

//
// Textbox. Spawns a FormWidget when asked.
class Textbox : public Widget {
public:
  Textbox(std::unique_ptr<Window> window);
  Textbox(Opts opts, Dimensions dimensions);
  ~Textbox() override {}

  Textbox *WithCb(std::function<void(absl::string_view)> recv_cb);
  Textbox *WithTemplate(std::function<std::string(std::string)> tmpl);

  void Update(std::string s);

  // Returns true if this widget consumed the event.
  bool Process() override;

private:
  void PersistForm();
  void CancelForm();

  // Optional recv_cb_.
  absl::optional<std::function<void(absl::string_view)>> recv_cb_;
  absl::optional<std::function<std::string(std::string)>> tmpl_;
  std::string content_;
  std::unique_ptr<FormWidget> form_{nullptr};
};

class Gridbox : public Widget {
public:
  Gridbox(Opts opts, Dimensions dimensions, int num_lines, int num_cols);
  ~Gridbox() override {}

  template <typename T, typename... Args> //
  T *Add(int y, int x, Args... args) {
    auto gridbox_dims = window_->GetDimensions();
    auto sub_dims = Dimensions{
        .nlines = 3,
        .ncols = 10,
        .begin_y = gridbox_dims.Height() / num_lines_ * y,
        .begin_x = gridbox_dims.Width() / num_cols_ * x,
    };
    auto window = absl::make_unique<Window>(
        sub_dims, window_->GetOpts(), BorderStyle::kThin,
        derwin(**window_, sub_dims.nlines, sub_dims.ncols, sub_dims.begin_y,
               sub_dims.begin_x));
    widgets_array_[y][x] = std::make_unique<T>(args..., std::move(window));
    return Get<T>(y, x);
  }

  template <typename T> //
  T *Get(int y, int x) {
    return static_cast<T *>(widgets_array_[y][x].get());
  }

  // Returns true if this widget consumed the event.
  bool Process() override;

private:
  const int num_lines_;
  const int num_cols_;
  std::vector<std::vector<std::unique_ptr<Widget>>> widgets_array_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_WIDGET_H_
