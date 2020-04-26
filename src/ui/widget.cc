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

#include "absl/strings/ascii.h"
#include "absl/strings/str_format.h"
#include "absl/time/clock.h"
#include <form.h>
#include <iostream>
#include <ncurses.h>

namespace latis {
namespace ui {

Widget::Widget(Opts opts, Dimensions dimensions)
    : dimensions_(dimensions), opts_(opts),
      window_(absl::make_unique<Window>(dimensions, opts)) {}

void Widget::Clear() { window_->Clear(); }

void Widget::Debug(absl::string_view txt) {
  if (opts_.show_debug_textbox) {
    window_->Print(0, 0, txt);
  }
}

FormWidget::FormWidget(Opts opts, Dimensions dimensions,
                       absl::string_view placeholder)
    : Widget(opts, dimensions) {
  // The general flow of control of a form program looks like this:
  //   1. Create the form fields, using new_field().
  Debug("starting form");

  fields_[0] =
      new_field(/*height=*/1, /*width=*/window_->Width() - 4, /*toprow=*/1,
                /*leftcol=*/2, /*offscreen=*/0, /*numbuf=*/0);
  assert(errno == E_OK);
  assert(fields_[0] != nullptr);

  Debug("Created all fields");

  assert(E_OK == set_field_back(fields_[0], A_UNDERLINE)); // Print a line
  assert(E_OK == field_opts_off(fields_[0], O_AUTOSKIP | O_STATIC));

  set_field_buffer(fields_[0], 0, std::string(placeholder).data());

  // bold and colorful
  const int input_colors = 1;
  start_color();
  init_pair(input_colors, COLOR_CYAN, COLOR_BLACK);
  assert(E_OK == set_field_fore(fields_[0], A_BOLD | COLOR_PAIR(input_colors)));

  Debug("set all field options");

  //   2. Create the form using new_form().
  form_ = new_form(fields_);
  set_form_fields(form_, fields_);
  assert(form_fields(form_) == fields_);
  assert(field_count(form_) == 1);
  assert(form_ != nullptr);
  assert(errno == E_OK);

  Debug("did new_form()");

  int rows;
  int cols;
  scale_form(form_, &rows, &cols);

  assert(E_OK == set_form_sub(form_, **window_));
  Debug("set form sub");

  //   3. Post the form using post_form().
  assert(E_OK == post_form(form_));

  set_current_field(form_, fields_[0]);

  Debug("posted form");

  //   4. Refresh the screen.
  window_->Refresh();
}

void FormWidget::BubbleCh(int ch) {
  //   5. Process user requests via an input loop.
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
}

FormWidget::~FormWidget() {
  // We don't want to clean up window_ via unpost_form or free_field, since
  // window_ already gets cleaned up by Window::~Window().
}

void FormWidget::BubbleEvent(const MEVENT &event) {
  // Do nothing. FormWidgets are fully driven by character input.
}

std::string FormWidget::Extract() {
  assert(field_status(fields_[0]) == true);
  return std::string(
      absl::StripTrailingAsciiWhitespace(field_buffer(fields_[0], 0)));
}

Textbox::Textbox(Opts opts, Dimensions dimensions) : Widget(opts, dimensions) {}

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

void Textbox::BubbleCh(int ch) {
  if (form_ != nullptr) {
    if (ch == 10) {
      // send the form the <enter> first.
      form_->BubbleCh(ch);

      std::string s = form_->Extract();
      form_ = nullptr;
      Update(s);
      if (recv_cb_.has_value()) {
        recv_cb_.value()(s);
      }
    } else {
      form_->BubbleCh(ch);
    }
  }
}

void Textbox::BubbleEvent(const MEVENT &event) {
  if (!window_->Contains(event.y, event.x)) {
    return;
  }
  if (form_ != nullptr) {
    form_->BubbleEvent(event);
  } else if (form_ == nullptr && recv_cb_.has_value() &&
             event.bstate & BUTTON1_CLICKED) {
    form_ = absl::make_unique<FormWidget>(opts_, dimensions_, content_);
  }
}

} // namespace ui
} // namespace latis
