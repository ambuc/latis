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

void Widget::Clear() {
  window_->Clear();
  window_->Refresh();
}

Textbox::Textbox(Dimensions dimensions, Opts opts,
                 absl::optional<std::function<void(absl::string_view)>> recv_cb)
    : Widget(dimensions, opts), recv_cb_(std::move(recv_cb)) {}

void Textbox::Update(absl::string_view s) {
  window_->Print(1, 2, s);
  window_->Refresh();
}

void Textbox::BubbleCh(int ch) {
  if (form_ != nullptr && ch == 10) {
    form_ = nullptr;
    Update("Entered. Form is gone.");
  }
}

void Textbox::BubbleEvent(const MEVENT &event) {
  if (!window_->Contains(event.y, event.x)) {
    return;
  }
  if (event.bstate & BUTTON1_CLICKED) {
    Update("Becoming form");
    form_ = absl::make_unique<Form>(window_.get(), opts_);
  }
}

} // namespace ui
} // namespace latis
