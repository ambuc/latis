
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

TextWidget *TextWidget::WithCb(Cb recv_cb) {
  recv_cb_ = recv_cb;
  return this;
}

TextWidget *TextWidget::WithTemplate(TmplCb tmpl) {
  tmpl_ = tmpl;
  return this;
}

void TextWidget::UpdateUnderlyingContent(std::string s) {
  static auto id = [](std::string s) { return s; };

  Debug(absl::StrFormat("TextWidget::UpdateUnderlyingContent(%s)", s));
  underlying_content_ = s;
  display_content_ = tmpl_.value_or(id)(underlying_content_);
  FormatAndFlushToWindow(display_content_);
}

void TextWidget::UpdateDisplayContent(std::string s) {
  Debug(absl::StrFormat("TextWidget::UpdateDisplayContent(%s)", s));
  display_content_ = s;
  FormatAndFlushToWindow(display_content_);
}

bool TextWidget::Process(int ch) {
  Debug(absl::StrFormat("TextWidget::Process(%c)", ch));

  bool did_process = false;
  if (form_ != nullptr) {
    did_process |= form_->Process(ch);
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

  if (CanHaveForm() && ch == KEY_ENTER) {
    auto dims = window_->GetDimensions();
    form_ = absl::make_unique<FormWidget>( //
        window_->GetDerwin(Dimensions{
            .nlines = dims.nlines,
            .ncols = dims.ncols,
            .begin_y = 0,
            .begin_x = 0,
        }),
        underlying_content_);
    window_->Refresh();
    return true;
  }

  return did_process;
}

bool TextWidget::CanHaveForm() {
  return form_ == nullptr && recv_cb_.has_value();
}

void TextWidget::PersistForm() {
  Debug("TextWidget::PersistForm");

  assert(form_ != nullptr);
  UpdateUnderlyingContent(form_->Extract());
  form_ = nullptr;

  assert(recv_cb_.has_value());
  UpdateDisplayContent(
      recv_cb_.value()(underlying_content_).value_or(display_content_));

  window_->Refresh();
}

void TextWidget::CancelForm() {
  Debug("TextWidget::CancelForm");

  assert(form_ != nullptr);
  form_ = nullptr;
}

void TextWidget::FormatAndFlushToWindow(absl::string_view s) {
  Debug(absl::StrFormat("TextWidget::FormatAndFlushToWindow(%s)", s));
  const auto style = window_->GetStyle();
  const auto dims = window_->GetDimensions();

  std::string to_print = std::string(s);

  int y_offset = 0;
  int x_offset = 0;
  if (style.border_style != BorderStyle::kBorderStyleNone) {
    y_offset += 1;
    x_offset += 1;
  }
  y_offset += style.ypad;
  x_offset += style.xpad;

  int width = dims.ncols;
  if (style.border_style != BorderStyle::kBorderStyleNone) {
    width -= 2;
  }
  width -= style.xpad;
  if (width < int(to_print.size())) {
    to_print.resize(width - 3);
    to_print.append("...");
  }

  if (style.halign == HorizontalAlignment::kCenter) {
    x_offset += (width / 2) - (int(to_print.size()) / 2) + 1;
  } else if (style.halign == HorizontalAlignment::kRight) {
    x_offset += width - int(to_print.size()) - 1;
  }

  // TODO(ambuc): impl valign

  window_->Print(y_offset, x_offset, to_print);
  window_->Refresh();
}

} // namespace ui
} // namespace latis
