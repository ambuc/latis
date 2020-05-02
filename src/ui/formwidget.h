
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
#ifndef SRC_UI_FORMWIDGET_H_
#define SRC_UI_FORMWIDGET_H_

#include "src/ui/widget.h"

#include "absl/memory/memory.h"

namespace latis {
namespace ui {

// Forms are hard.
// https://invisible-island.net/ncurses/ncurses-intro.html#form
class FormWidget : public Widget {
public:
  FormWidget(std::unique_ptr<Window> window, absl::string_view placeholder);
  ~FormWidget() override;

  // Returns true if this widget consumed the event.
  bool Process(int ch, const MEVENT &event, bool is_mouse) override;

  std::string Extract();

private:
  FORM *form_{nullptr};
  // Inherently just a single field.
  FIELD *fields_[2]{nullptr, nullptr};
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_FORMWIDGET_H_
