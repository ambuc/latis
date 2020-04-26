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

#include "src/ui/app.h"

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

#include <form.h>
#include <iostream>
#include <ncurses.h>

namespace latis {
namespace ui {

App::App() {
  setlocale(LC_ALL, "");

  initscr();

  halfdelay(1000 / 60); // 60 FPS
  keypad(stdscr, true); // enable mouse.
  mousemask(ALL_MOUSE_EVENTS, NULL);

  cbreak();
  noecho();
  clear();

  refresh();
}

App::~App() { endwin(); }

std::shared_ptr<Textbox> App::AddTextbox(
    absl::string_view title, Dimensions dimensions, Opts opts,
    absl::optional<std::function<void(absl::string_view)>> recv_cb) {
  auto textbox =
      std::make_shared<Textbox>(dimensions, opts, std::move(recv_cb));
  widgets_[title] = textbox;
  return textbox;
}

std::shared_ptr<TextboxWithTemplate> App::AddTextboxWithTemplate(
    absl::string_view title, Dimensions dimensions, Opts opts,
    std::function<std::string(std::string)> tmpl,
    absl::optional<std::function<void(absl::string_view)>> recv_cb) {
  auto textbox_with_template = std::make_shared<TextboxWithTemplate>(
      dimensions, opts, tmpl, std::move(recv_cb));
  widgets_[title] = textbox_with_template;
  return textbox_with_template;
}

std::shared_ptr<Widget> App::Get(absl::string_view title) {
  if (const auto it = widgets_.find(title); it == widgets_.end()) {
    return nullptr;
  } else {
    return it->second;
  }
}

void App::Remove(absl::string_view title) { widgets_.erase(title); }

void App::BubbleCh(int ch) {
  for (auto &[_, w] : widgets_) {
    w->BubbleCh(ch);
  }
}

void App::BubbleEvent(const MEVENT &event) {
  for (auto &[_, w] : widgets_) {
    w->BubbleEvent(event);
  }
}

} // namespace ui
} // namespace latis
