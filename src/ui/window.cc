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

#include "absl/memory/memory.h"
#include "absl/strings/str_format.h"
#include "absl/time/clock.h"

#include <ncurses.h>

namespace latis {
namespace ui {

Window::Window(Dimensions dimensions, Opts opts, WINDOW *window)
    : dimensions_(dimensions), opts_(opts), border_style_(BorderStyle::kThin),
      ptr_(window) {
  PrintPermanentComponents();
  Refresh();
}

std::unique_ptr<Window> Window::GetDerwin(Dimensions dimensions, Opts opts) {
  return absl::make_unique<Window>(dimensions, opts,
                                   derwin(ptr_, dimensions.nlines,
                                          dimensions.ncols, dimensions.begin_y,
                                          dimensions.begin_x));
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

  assert(mvwprintw(ptr_, y, x, puts.c_str()) == OK);
  Refresh();
}

void Window::Refresh() {
  //
  assert(OK == wrefresh(ptr_));
}

void Window::Clear() {
  assert(OK == wclear(ptr_));
  PrintPermanentComponents();
  Refresh();
}

Dimensions Window::GetDimensions() const { return dimensions_; }
Opts Window::GetOpts() const { return opts_; }

Window::~Window() {
  assert(wclear(ptr_) == OK);
  Refresh();
  delwin(ptr_);
}

void Window::PrintPermanentComponents() {
  if (opts_.show_borders) {
    // https://invisible-island.net/ncurses/man/curs_border_set.3x.html
    // https://invisible-island.net/ncurses/man/curs_add_wch.3x.html
    switch (border_style_) {
    case (BorderStyle::kThin): {
      assert(wborder_set(ptr_, WACS_VLINE, WACS_VLINE, WACS_HLINE, WACS_HLINE,
                         WACS_ULCORNER, WACS_URCORNER, WACS_LLCORNER,
                         WACS_LRCORNER) == OK);
      break;
    }
    case (BorderStyle::kThick): {
      assert(wborder_set(ptr_, WACS_T_VLINE, WACS_T_VLINE, WACS_T_HLINE,
                         WACS_T_HLINE, WACS_T_ULCORNER, WACS_T_URCORNER,
                         WACS_T_LLCORNER, WACS_T_LRCORNER) == OK);
      break;
    }
    case (BorderStyle::kDouble): {
      assert(wborder_set(ptr_, WACS_D_VLINE, WACS_D_VLINE, WACS_D_HLINE,
                         WACS_D_HLINE, WACS_D_ULCORNER, WACS_D_URCORNER,
                         WACS_D_LLCORNER, WACS_D_LRCORNER) == OK);
      break;
    }
    default: {
      // none
    }
    }
  }
}

} // namespace ui
} // namespace latis
