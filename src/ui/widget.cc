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

Widget::Widget(std::unique_ptr<Window> window) : window_(std::move(window)) {
  Debug(absl::StrFormat("Widget::Widget(%p)", window.get()));
  window_->Refresh();
}

void Widget::Clear() {
  Debug("Widget::Clear()");
  window_->Clear();
  window_->Refresh();
}

} // namespace ui
} // namespace latis
