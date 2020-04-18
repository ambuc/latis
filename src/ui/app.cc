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

// WINDOW

Window::Window(int nlines, int ncols, int begin_y, int begin_x)
    : nlines_(nlines), ncols_(ncols), begin_y_(begin_y), begin_x_(begin_x),
      ptr_(newwin(nlines_, ncols_, begin_y_, begin_x_)) {
  wborder(ptr_, '|', '|', '-', '-', '+', '+', '+', '+');
  mvwhline(ptr_, 2, 0, '=', ncols_);
  if (true) {
    std::string dimensions = absl::StrFormat("%dx%d", nlines_, ncols_);
    mvwprintw(ptr_, 0, 0, dimensions.c_str());
  }
  wrefresh(ptr_);
}

Window::~Window() { delwin(ptr_); }

// APP

App::App(Opts opts) : opts_(opts) {
  setlocale(LC_ALL, "");

  initscr();
  cbreak();
  noecho();
  clear();

  refresh();

  {
    auto w = absl::make_unique<Window>(10, 20, 0, 0);
    mvwprintw(*w, 1, 2, "foo");
    wrefresh(*w);

    int ch;
    while ((ch = getch()) != 'q') {
      switch (ch) {
      case 'p': {
        mvwprintw(*w, 1, 0, "BAR");
        wrefresh(*w);
        break;
      }
      default: {
        // do nothing
      }
      }
    }
  }
  clrtobot();
  refresh();
}

} // namespace ui
} // namespace latis
