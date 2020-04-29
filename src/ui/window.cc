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

Window::Window(Dimensions dimensions, Opts opts, WINDOW *window)
    : dimensions_(dimensions), opts_(opts), ptr_(window) {
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

Dimensions Window::GetDimensions() const { return dimensions_; }

Window::~Window() {
  wclear(ptr_);
  Refresh();
  delwin(ptr_);
}

void Window::PrintPermanentComponents() {
  if (opts_.show_borders) {
    // https://invisible-island.net/ncurses/man/curs_border_set.3x.html
    // https://invisible-island.net/ncurses/man/curs_add_wch.3x.html
    wborder_set(ptr_, WACS_D_VLINE, WACS_D_VLINE, WACS_D_HLINE, WACS_D_HLINE,
                WACS_D_ULCORNER, WACS_D_URCORNER, WACS_D_LLCORNER,
                WACS_D_LRCORNER);
  }
}

} // namespace ui
} // namespace latis
