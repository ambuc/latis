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

#include "src/ui/widget.h"

#include "src/ui/window.h"

#include "absl/strings/ascii.h"
#include "absl/strings/str_format.h"
#include "absl/time/clock.h"
#include <form.h>
#include <iostream>

namespace latis {
namespace ui {

Widget::Widget(std::unique_ptr<Window> window) : window_(std::move(window)) {}

void Widget::Clear() { window_->Clear(); }

void Widget::Debug(absl::string_view txt) { window_->Print(0, 0, txt); }

FormWidget::FormWidget(std::unique_ptr<Window> window,
                       absl::string_view placeholder)
    : Widget(std::move(window)) {
  // The general flow of control of a form program looks like this:
  //   1. Create the form fields, using new_field().

  fields_[0] =
      new_field(/*height=*/1, /*width=*/window_->GetDimensions().Width() - 4,
                /*toprow=*/1,
                /*leftcol=*/2, /*offscreen=*/0, /*numbuf=*/0);
  assert(errno == E_OK);
  assert(fields_[0] != nullptr);

  assert(E_OK == set_field_back(fields_[0], A_UNDERLINE)); // Print a line
  assert(E_OK == field_opts_off(fields_[0], O_AUTOSKIP | O_STATIC));

  set_field_buffer(fields_[0], 0, std::string(placeholder).data());

  // bold and colorful
  const int input_colors = 1;
  start_color();
  init_pair(input_colors, COLOR_CYAN, COLOR_BLACK);
  assert(E_OK == set_field_fore(fields_[0], A_BOLD | COLOR_PAIR(input_colors)));

  //   2. Create the form using new_form().
  form_ = new_form(fields_);
  set_form_fields(form_, fields_);
  assert(form_fields(form_) == fields_);
  assert(field_count(form_) == 1);
  assert(form_ != nullptr);
  assert(errno == E_OK);

  int rows;
  int cols;
  scale_form(form_, &rows, &cols);

  assert(E_OK == set_form_sub(form_, **window_));

  //   3. Post the form using post_form().
  assert(E_OK == post_form(form_));

  set_current_field(form_, fields_[0]);

  //   4. Refresh the screen.
  window_->Refresh();
}

bool FormWidget::Process(int ch) {
  switch (ch) {
  case KEY_ENTER:
  case 10:
    form_driver(form_, REQ_NEXT_FIELD);
    break;
  case KEY_BACKSPACE:
  case 127:
    form_driver(form_, REQ_DEL_PREV);
    break;
  case KEY_LEFT:
    form_driver(form_, REQ_PREV_CHAR);
    break;
  case KEY_RIGHT:
    form_driver(form_, REQ_NEXT_CHAR);
    break;
  default:
    form_driver(form_, ch);
    break;
  }
  window_->Refresh();
  return true;
}

FormWidget::~FormWidget() {
  // We don't want to clean up window_ via unpost_form or free_field, since
  // window_ already gets cleaned up by Window::~Window().
}

std::string FormWidget::Extract() {
  assert(field_status(fields_[0]) == true);
  return std::string(
      absl::StripTrailingAsciiWhitespace(field_buffer(fields_[0], 0)));
}

Textbox::Textbox(std::unique_ptr<Window> window) : Widget(std::move(window)) {}

Textbox::Textbox(Opts opts, Dimensions dimensions)
    : Widget(absl::make_unique<Window>(dimensions, opts)) {}

Textbox *Textbox::WithCb(std::function<void(absl::string_view)> recv_cb) {
  recv_cb_ = recv_cb;
  return this;
}

Textbox *Textbox::WithTemplate(std::function<std::string(std::string)> tmpl) {
  tmpl_ = tmpl;
  return this;
}

void Textbox::Update(std::string s) {
  content_ = s;
  window_->Print(1, 2, tmpl_.has_value() ? tmpl_.value()(content_) : content_);
}

bool Textbox::Process(int ch) {
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

  if (form_ == nullptr && recv_cb_.has_value()) {
    // if there's no form and this textbox is form-enabled:
    if (ch != KEY_MOUSE) {
      return false;
    }
    MEVENT event;
    if (getmouse(&event) != OK) {
      return did_process;
    }
    if (!wenclose(**window_, event.y, event.x)) {
      ungetmouse(&event);
      return false;
    }
    if (event.bstate &
        (BUTTON1_PRESSED | BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED)) {
      auto dims = window_->GetDimensions();
      form_ = absl::make_unique<FormWidget>(window_->GetDerwin(
                                                Dimensions{
                                                    .nlines = dims.nlines,
                                                    .ncols = dims.ncols,
                                                    .begin_y = 0,
                                                    .begin_x = 0,
                                                },
                                                window_->GetOpts(),
                                                BorderStyle::kThin),
                                            content_);
      return true;
    }
  }

  return did_process;
}

void Textbox::PersistForm() {
  assert(form_ != nullptr);

  std::string s = form_->Extract();
  form_ = nullptr;

  Update(s);

  if (recv_cb_.has_value()) {
    recv_cb_.value()(s);
  }
}

void Textbox::CancelForm() {
  assert(form_ != nullptr);
  form_ = nullptr;
  Update(content_);
}

Gridbox::Gridbox(Opts opts, Dimensions dimensions, int num_lines, int num_cols)
    : Widget(absl::make_unique<Window>(dimensions, opts)),
      num_lines_(num_lines), num_cols_(num_cols), widgets_array_() {
  widgets_array_.resize(num_cols_);
  for (std::vector<std::unique_ptr<Widget>> &v : widgets_array_) {
    v.resize(num_lines_);
  }
}

bool Gridbox::Process(int ch) {
  bool did_process = false;
  for (std::vector<std::unique_ptr<Widget>> &v : widgets_array_) {
    for (std::unique_ptr<Widget> &cell : v) {
      if (cell != nullptr) {
        if (cell->Process(ch)) {
          did_process = true;
        }
      }
    }
  }
  return did_process;
}

} // namespace ui
} // namespace latis
