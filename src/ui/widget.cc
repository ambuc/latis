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

#include "absl/strings/str_format.h"
#include "absl/time/clock.h"
#include <form.h>
#include <iostream>
#include <ncurses.h>

namespace latis {
namespace ui {

Widget::Widget(Dimensions dimensions, Opts opts)
    : dimensions_(dimensions), opts_(opts),
      window_(absl::make_unique<Window>(dimensions, opts)) {}

void Widget::Clear() { window_->Clear(); }

void Widget::Debug(absl::string_view txt) {
  if (opts_.show_debug_textbox) {
    window_->Print(0, 0, txt);
  }
}

FormWidget::FormWidget(Dimensions dimensions, Opts opts)
    : Widget(dimensions, opts) {
  Debug("starting form");

  fields_[0] =
      new_field(/*height=*/1, /*width=*/window_->Width() - 4, /*toprow=*/1,
                /*leftcol=*/2, /*offscreen=*/0, /*numbuf=*/0);
  assert(errno == E_OK);
  assert(fields_[0] != nullptr);

  Debug("Created all fields");

  assert(E_OK == set_field_back(fields_[0], A_UNDERLINE)); // Print a line
  assert(E_OK ==
         field_opts_off(fields_[0], O_AUTOSKIP)); // Don't skip when full

  Debug("set all field options");

  form_ = new_form(fields_);
  set_form_fields(form_, fields_);
  assert(form_fields(form_) == fields_);
  assert(field_count(form_) == 1);
  assert(form_ != nullptr);

  if (errno != E_OK) {
    assert(errno != E_BAD_ARGUMENT);
    assert(errno != E_BAD_STATE);
    assert(errno != E_NOT_POSTED);
    assert(errno != E_NOT_CONNECTED);
    assert(errno != E_CONNECTED);
    assert(errno != E_NO_ROOM);
    assert(errno != E_POSTED);
    assert(errno != E_SYSTEM_ERROR);
    assert(errno == E_OK);
  }

  Debug("did new_form()");

  int rows;
  int cols;
  scale_form(form_, &rows, &cols);

  // assert(E_OK == set_form_win(form_, **window_)); // Link form to window.
  // Debug("set form win");
  assert(E_OK == set_form_sub(form_, **window_));
  Debug("set form sub");

  if (const int result = post_form(form_); result != E_OK) {
    assert(result != E_BAD_ARGUMENT);
    assert(result != E_BAD_STATE);
    assert(result != E_NOT_POSTED);
    assert(result != E_NOT_CONNECTED);
    assert(result != E_NO_ROOM);
    assert(result != E_POSTED);
    assert(result != E_SYSTEM_ERROR);
    assert(result == E_OK);
    Debug(std::to_string(result));
  }

  set_current_field(form_, fields_[0]);

  Debug("posted form");
}

std::string FormWidget::Extract() {
  assert(field_status(fields_[0]) == true);

  return "TODO";
}

FormWidget::~FormWidget() {
  // We don't want to clean up window_ via unpost_form or free_field, since
  // window_ already gets cleaned up by Window::~Window().
}

void FormWidget::BubbleCh(int ch) {
  Debug(absl::StrFormat("Handling FormWidget::bubblech %s", ch));
  // ch = wgetch(**window_);

  switch (ch) {
  case KEY_DOWN:
    form_driver(form_, REQ_NEXT_FIELD);
    form_driver(form_, REQ_END_LINE);
    break;
  case KEY_UP:
    form_driver(form_, REQ_PREV_FIELD);
    form_driver(form_, REQ_END_LINE);
    break;
  default:
    form_driver(form_, ch);
    break;
  }
  window_->Refresh();
}

void FormWidget::BubbleEvent(const MEVENT &event) {
  // Do nothing. FormWidgets are fully driven by character input.
}

Textbox::Textbox(Dimensions dimensions, Opts opts,
                 absl::optional<std::function<void(absl::string_view)>> recv_cb)
    : Widget(dimensions, opts), recv_cb_(std::move(recv_cb)) {}

void Textbox::Update(absl::string_view s) { window_->Print(1, 2, s); }

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
    Debug("Becoming form");
    form_ = absl::make_unique<FormWidget>(dimensions_, opts_);
  }
}

} // namespace ui
} // namespace latis
