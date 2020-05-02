
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

#include "src/ui/textwidget.h"

namespace latis {
namespace ui {

TextWidget::TextWidget(std::unique_ptr<Window> window)
    : Widget(std::move(window)) {
  Debug(absl::StrFormat("TextWidget::TextWidget(%p)", window.get()));
}

TextWidget::TextWidget(Dimensions dimensions)
    : Widget(absl::make_unique<Window>(dimensions)) {}

TextWidget *TextWidget::WithCb(std::function<void(absl::string_view)> recv_cb) {
  recv_cb_ = recv_cb;
  return this;
}

TextWidget *
TextWidget::WithTemplate(std::function<std::string(std::string)> tmpl) {
  tmpl_ = tmpl;
  return this;
}

void TextWidget::Update(std::string s) {
  Debug(absl::StrFormat("TextWidget::Update(%s)", s));

  content_ = s;
  window_->Print(1, 2, tmpl_.has_value() ? tmpl_.value()(content_) : content_);
  window_->Refresh();
}

bool TextWidget::Process(int ch, const MEVENT &event, bool is_mouse) {
  Debug(absl::StrFormat("TextWidget::Process(%c)", ch));

  bool did_process = false;
  if (form_ != nullptr) {
    did_process |= form_->Process(ch, event, is_mouse);
  }

  if (form_ != nullptr) {
    switch (ch) {
    case (KEY_ENTER):
    case (10): {
      PersistForm();
      return true;
    }
    // KEY_ESC
    case (27): {
      CancelForm();
      return true;
    }
    default: {
      break;
      //
    }
    }
  }

  if (CanHaveForm() && is_mouse && ch == KEY_MOUSE &&
      wenclose(**window_, event.y, event.x)) {
    if (event.bstate &
        (BUTTON1_PRESSED | BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED)) {
      auto dims = window_->GetDimensions();
      form_ = absl::make_unique<FormWidget>( //
          window_->GetDerwin(Dimensions{
              .nlines = dims.nlines,
              .ncols = dims.ncols,
              .begin_y = 0,
              .begin_x = 0,
          }),
          content_);
      window_->Refresh();
      return true;
    }
  }

  return did_process;
}

bool TextWidget::CanHaveForm() {
  return form_ == nullptr && recv_cb_.has_value();
}

void TextWidget::PersistForm() {
  Debug("TextWidget::PersistForm");

  assert(form_ != nullptr);

  std::string s = form_->Extract();
  form_ = nullptr;

  Update(s);

  assert(recv_cb_.has_value());
  recv_cb_.value()(s);
}

void TextWidget::CancelForm() {
  Debug("TextWidget::CancelForm");

  assert(form_ != nullptr);
  form_ = nullptr;
  Update(content_);
}

} // namespace ui
} // namespace latis
