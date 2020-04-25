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
#include <form.h>
#include <ncurses.h>

namespace latis {
namespace ui {

Widget::Widget(Dimensions dimensions, Opts opts)
    : dimensions_(dimensions), opts_(opts) {}

// Form::Form(WINDOW *window_ptr) : window_ptr_(window_ptr) {
//   assert(window_ptr_ != nullptr);
//
//   field_[0] = new_field(1, 10, 0, 0, 0, 0);
//   set_field_back(field_[0], A_UNDERLINE);
//   field_opts_off(field_[0], O_AUTOSKIP);
//   assert(field_[0] != nullptr);
//   set_field_buffer(field_[0], 0, "label1");
//
//   assert(form_ == nullptr);
//   form_ = new_form(field_);
//   assert(form_ != nullptr);
//   keypad(window_ptr_, TRUE);
//   // int rows = 1;
//   // int cols = 10;
//   // scale_form(form_, &rows, &cols);
//   // set_form_win(form_, window_ptr_);
//   // auto dw = derwin(window_ptr_, rows, cols, 0, 0);
//   // set_form_sub(form_, dw);
//   // post_form(form_);
//   // wrefresh(dw);
//   // wrefresh(window_ptr_);
// }
//
// Form::~Form() {
//   if (form_ != nullptr) {
//     unpost_form(form_);
//     free_form(form_);
//     // form_ = nullptr;
//   }
//   if (field_[0] != nullptr) {
//     free_field(field_[0]);
//     // field_[0] = nullptr;
//   }
// }
//
// void Form::BubbleCh(int ch) {
//   if (form_ != nullptr) {
//     switch (ch) {
//     case KEY_DOWN:
//       form_driver(form_, REQ_NEXT_FIELD);
//       form_driver(form_, REQ_END_LINE);
//       break;
//     case KEY_UP:
//       form_driver(form_, REQ_PREV_FIELD);
//       form_driver(form_, REQ_END_LINE);
//       break;
//     default:
//       form_driver(form_, ch);
//       break;
//     }
//   }
// }

Textbox::Textbox(Dimensions dimensions, Opts opts,
                 absl::optional<std::function<void(absl::string_view)>> recv_cb)
    : Widget(dimensions, opts), recv_cb_(std::move(recv_cb)),
      window_(absl::make_unique<Window>(dimensions, opts)) {}

void Textbox::Update(absl::string_view s) {
  window_->Print(1, 2, s);
  window_->Refresh();
}

void Textbox::Clear() {
  window_->Clear();
  window_->Refresh();
}

void Textbox::BubbleCh(int ch) {
  echo();
  // if (form_ != nullptr) {
  //   form_->BubbleCh(ch);
  // }
}

void Textbox::BubbleEvent(const MEVENT &event) {
  if (!window_->Contains(event.y, event.x)) {
    return;
  }
  // if (event.bstate & BUTTON1_CLICKED) {
  //   Update("Becoming form");
  //   BecomeFormIfNotAlready();
  // } else if (event.bstate & BUTTON1_DOUBLE_CLICKED) {
  //   BecomeDisplayIfNotAlready();
  // }
}

// void Textbox::BecomeFormIfNotAlready() {
//   if (form_ == nullptr) {
//     Update("Becoming form...");
//     form_ = absl::make_unique<Form>(**window_);
//   }
// }

// void Textbox::BecomeDisplayIfNotAlready() {
//   if (form_ != nullptr) {
//     form_.reset(nullptr);
//   }
// }

} // namespace ui
} // namespace latis
