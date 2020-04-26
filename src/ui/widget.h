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
#include <ncurses.h>

namespace latis {
namespace ui {

class Widget {
public:
  Widget(Dimensions dimensions, Opts opts);
  virtual ~Widget() = default;

  void Clear();

  virtual void BubbleCh(int ch) = 0;
  virtual void BubbleEvent(const MEVENT &event) = 0;

protected:
  void Debug(absl::string_view txt);

  const Dimensions dimensions_;
  const Opts opts_;
  const std::unique_ptr<Window> window_;
};

// Forms are hard.
// TODO(ambuc): Make it so selecting one field unselects the others.
// TODO(ambuc): Make it so a FormWidget spawns with the current text.
// https://invisible-island.net/ncurses/ncurses-intro.html#form
class FormWidget : public Widget {
public:
  FormWidget(Dimensions dimensions, Opts opts);
  ~FormWidget() override;

  void BubbleCh(int ch) override;
  void BubbleEvent(const MEVENT &event) override;

  std::string Extract();

private:
  FORM *form_{nullptr};
  FIELD *fields_[2]{nullptr, nullptr};
};

// Textbox. Spawns a FormWidget when asked.
class Textbox : public Widget {
public:
  Textbox(Dimensions dimensions, Opts opts,
          absl::optional<std::function<void(absl::string_view)>> recv_cb);
  ~Textbox() override {}

  void Update(absl::string_view s);
  void BubbleCh(int ch) override;
  void BubbleEvent(const MEVENT &event) override;

private:
  const absl::optional<std::function<void(absl::string_view)>> recv_cb_;

  std::unique_ptr<FormWidget> form_{nullptr};
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_WIDGET_H_
