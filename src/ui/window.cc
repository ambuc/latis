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

#include "src/ui/window.h"

#include "absl/strings/str_format.h"

#include <ncurses.h>

namespace latis {
namespace ui {

Window::Window(Dimensions dimensions, Opts opts)
    : dimensions_(dimensions), opts_(opts),
      ptr_(newwin(dimensions_.nlines, dimensions_.ncols, dimensions_.begin_y,
                  dimensions_.begin_x)) {
  PrintPermanentComponents();
  Refresh();
}

void Window::Print(int y, int x, absl::string_view s) {
  PrintPermanentComponents();

  std::string puts = std::string(s);
  // two edges and two padding
  if (int(s.size()) > dimensions_.ncols - 4) {
    // two edges, two padding, and three ellipses.
    puts.resize(dimensions_.ncols - 4 - 3);
    puts.append("...");
  } else {
    // Fill the rest with spaces.
    puts.resize(dimensions_.ncols - 4, ' ');
  }

  mvwprintw(ptr_, y, x, puts.c_str());
  Refresh();
}

void Window::Refresh() { wrefresh(ptr_); }

void Window::Clear() {
  wclear(ptr_);
  PrintPermanentComponents();
  Refresh();
}

bool Window::Contains(int y, int x) const { return dimensions_.Contains(y, x); }

int Window::Width() const { return dimensions_.ncols; }
int Window::Height() const { return dimensions_.nlines; }

Window::~Window() {
  wclear(ptr_);
  Refresh();
  delwin(ptr_);
}

void Window::PrintPermanentComponents() {
  if (opts_.show_borders) {
    // wborder(ptr_, '|', '|', '-', '-', '+', '+', '+', '+');
    // wborder(ptr_, WACS_T_HLINE, '|', '-', '-', '+', '+', '+', '+');
    wborder_set(ptr_, WACS_VLINE, WACS_VLINE, WACS_HLINE, WACS_HLINE,
                WACS_ULCORNER, WACS_URCORNER, WACS_LLCORNER, WACS_LRCORNER);
  }
}

} // namespace ui
} // namespace latis
