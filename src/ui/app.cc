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

#include "src/ui/color.h"
#include "src/ui/common.h"

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

#include <iostream>
#include <ncurses.h>

namespace latis {
namespace ui {

App::App() {
  setlocale(LC_ALL, "");

  initscr();

  InitColors(); // see color.h

  halfdelay(1000 / 60);    // 60 FPS
  notimeout(stdscr, true); // no timeout, esc persists immediately

  keypad(stdscr, TRUE); // arrow key

  // Disable cursor
  assert(curs_set(0) != ERR);

  assert(cbreak() == OK);
  assert(noecho() == OK);
  assert(clear() == OK);

  assert(refresh() == OK);
}

App::~App() { endwin(); }

void App::RemoveAllWidgets() {
  active_ = std::make_unique<ActiveWidget>();
  widgets_.clear();
}

void App::Run() {
  bool should_run = true;
  int ch;
  do {

    ch = getch();
    ui::Debug(absl::StrFormat("Handling %c", ch));

    if (ch == KEY_RESIZE && resize_cb_.has_value()) {
      resize_cb_.value()();
      continue;
    }

    (*active_)->Process(ch);

    // Fallback -- if no one else processed it, I will.
    if (ch == int('q')) {
      should_run = false;
    }
  } while (should_run);
}

void App::RegisterResizeCallback(ResizeCb cb) { resize_cb_ = cb; }

} // namespace ui
} // namespace latis
