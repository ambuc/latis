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
#include <ncurses.h>

namespace latis {
namespace ui {

Textbox::Textbox(Dimensions dimensions,
                 std::function<void(absl::string_view)> recv_cb,
                 Widget::Options options)
    : options_(options), recv_cb_(std::move(recv_cb)),
      window_(absl::make_unique<Window>(
          dimensions, Window::Opts{
                          .show_dimensions = options_.debug_mode,
                      })) {}

void Textbox::Update(absl::string_view s) {
  window_->Print(1, 2, s);
  window_->Refresh();
}

void Textbox::Clear() {
  window_->Clear();
  window_->Refresh();
}

void Textbox::BubbleCh(int ch) {
  //
}

void Textbox::BubbleEvent(const MEVENT &event) {
  if (window_->Contains(event.y, event.x)) {
    Update(absl::StrFormat("%d,%d,%d,%d", event.bstate, event.x, event.y,
                           event.z));
  }
}

} // namespace ui
} // namespace latis
