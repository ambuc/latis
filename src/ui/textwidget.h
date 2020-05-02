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
#ifndef SRC_UI_TEXTWIDGET_H_
#define SRC_UI_TEXTWIDGET_H_

#include "src/ui/widget.h"

#include "src/ui/formwidget.h"

namespace latis {
namespace ui {

// TextWidget. Spawns a FormWidget when asked.
class TextWidget : public Widget {
public:
  TextWidget(std::unique_ptr<Window> window);
  TextWidget(Dimensions dimensions);
  ~TextWidget() override {}

  TextWidget *WithCb(std::function<void(absl::string_view)> recv_cb);
  TextWidget *WithTemplate(std::function<std::string(std::string)> tmpl);

  void Update(std::string s);

  // Returns true if this widget consumed the event.
  bool Process(int ch, const MEVENT &event, bool is_mouse) override;

private:
  bool CanHaveForm();
  void PersistForm();
  void CancelForm();

  // Optional recv_cb_.
  absl::optional<std::function<void(absl::string_view)>> recv_cb_;
  absl::optional<std::function<std::string(std::string)>> tmpl_;
  std::string content_;
  std::unique_ptr<FormWidget> form_{nullptr};
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_TEXTWIDGET_H_
