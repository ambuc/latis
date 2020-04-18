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

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

#include <iostream>
#include <ncurses.h>

namespace latis {
namespace ui {

App::App(Options options) : options_(options) {
  setlocale(LC_ALL, "");

  initscr();
  cbreak();
  noecho();
  clear();

  refresh();

  if (WINDOW *w = newwin(/*nlines=*/10, /*ncols=*/20, /*begin_y=*/0,
                         /*begin_x=*/0);
      w != nullptr) {
    box(w, 0, 0);
    wborder(w, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(w, 1, 2, "foo");
    if (options_.enable_window_dimensions) {
      int row, col;
      getmaxyx(stdscr, row, col);
      std::string dimensions = absl::StrFormat("%dx%d", row, col);
      mvwprintw(w, 0, 0, dimensions.c_str());
    }
    wrefresh(w);
  }
}

} // namespace ui
} // namespace latis
