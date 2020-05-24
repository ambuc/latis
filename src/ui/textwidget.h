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
  using Cb = std::function<absl::optional<std::string>(absl::string_view)>;
  using TmplCb = std::function<std::string(std::string)>;

  TextWidget(std::unique_ptr<Window> window);
  TextWidget(Dimensions dimensions);
  ~TextWidget() override {}

  TextWidget *WithCb(Cb recv_cb);
  TextWidget *WithTemplate(TmplCb tmpl);

  void UpdateUnderlyingContent(std::string s);
  void UpdateDisplayContent(std::string s);

  // Returns true if this widget consumed the event.
  bool Process(int ch) override;

  void Focus() override {
    Debug("FOCUSING on me, a textwidget.");
    window_->GetMutableStyle()->border_style = BorderStyle::kDouble;
    window_->GetMutableStyle()->border_color = COLOR_RED;
    window_->Refresh();
  }

  void UnFocus() override {
    Debug("UNFOCUSING on me, a textwidget.");
    window_->GetMutableStyle()->border_style = BorderStyle::kThin;
    window_->GetMutableStyle()->border_color = COLOR_WHITE;
    window_->Refresh();
  }

private:
  bool CanHaveForm();
  void PersistForm();
  void CancelForm();
  void FormatAndFlushToWindow(absl::string_view s);

  // Optional recv_cb_.
  absl::optional<Cb> recv_cb_;
  absl::optional<TmplCb> tmpl_;

  // the underlying content, i.e. "2+2"
  std::string underlying_content_;
  // the thing to print instead when not in form mode.
  std::string display_content_;

  std::unique_ptr<FormWidget> form_{nullptr};
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_TEXTWIDGET_H_
