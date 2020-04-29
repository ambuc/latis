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

#include <iostream>
#include <ncurses.h>

namespace latis {
namespace ui {

App::App(Opts opts) : opts_(opts) {
  setlocale(LC_ALL, "");

  initscr();

  halfdelay(1000 / 60);    // 60 FPS
  keypad(stdscr, true);    // enable mouse.
  notimeout(stdscr, true); // no timeout, esc persists immediately
  mousemask(ALL_MOUSE_EVENTS, NULL);

  cbreak();
  noecho();
  clear();

  refresh();
}

App::~App() { endwin(); }

void App::Remove(absl::string_view title) { widgets_.erase(title); }

void App::Run() {
  // New model. If you don't have anything to do with a keyboard/mouse event,
  // put it on the stack.
  while (true) {
    bool was_processed = false;
    for (auto &[_, w] : widgets_) {
      if (!was_processed && w->Process()) {
        was_processed = true;
      }
    }

    // Fallback -- if no one else processed it, I will.
    if (!was_processed) {
      // goodbye
      if (getch() == int('q')) {
        break;
      }
      was_processed = true;
    }

    assert(was_processed);
  }
}

} // namespace ui
} // namespace latis
