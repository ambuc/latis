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

#include <iostream>
#include <ncurses.h>

namespace latis {
namespace ui {

Window::Window(Dimensions dimensions, Style style, WINDOW *window)
    : dimensions_(dimensions), style_(style), ptr_(window) {
  assert(dimensions_.ncols > 1);

  // Debug( absl::StrFormat("Window::Window(%s,%p)", dimensions.ToString(),
  // window));

  PrintPermanentComponents();
}

std::unique_ptr<Window> Window::GetDerwin(Dimensions dimensions, Style style) {
  // Debug(absl::StrFormat("GetDerwin(%s)", dimensions.ToString()));

  return absl::make_unique<Window>(dimensions, style,
                                   derwin(ptr_, dimensions.nlines,
                                          dimensions.ncols, dimensions.begin_y,
                                          dimensions.begin_x));
}

void Window::Print(int y, int x, absl::string_view s) {
  // Debug(absl::StrFormat("Window::Print(%d,%d,%s)", y, x, s));
  // PrintPermanentComponents();
  Clear();
  assert(mvwprintw(ptr_, y, x, std::string(s).c_str()) == OK);
}

void Window::Refresh() {
  // Debug("Window::Refresh()");
  PrintPermanentComponents();
  assert(OK == wrefresh(ptr_));
}

void Window::Clear() {
  // Debug("Window::Clear()");

  assert(OK == wclear(ptr_));
}

Dimensions Window::GetDimensions() const { return dimensions_; }
Style Window::GetStyle() const { return style_; }

Window::~Window() {
  // Debug("Window::~Window()");

  assert(wclear(ptr_) == OK);
  delwin(ptr_);
}

void Window::PrintPermanentComponents() {
  // Debug("Window::PrintPermanentComponents()");

  // https://invisible-island.net/ncurses/man/curs_border_set.3x.html
  // https://invisible-island.net/ncurses/man/curs_add_wch.3x.html
  switch (style_.border_style) {
  case (BorderStyle::kThin): {
    switch (style_.corner_style) {
    case (CornerStyle::kPlus): {
      assert(wborder_set(ptr_, WACS_VLINE, WACS_VLINE, WACS_HLINE, WACS_HLINE,
                         WACS_PLUS, WACS_PLUS, WACS_PLUS, WACS_PLUS) == OK);
      break;
    }
    case (CornerStyle::kClosed): {
      assert(wborder_set(ptr_, WACS_VLINE, WACS_VLINE, WACS_HLINE, WACS_HLINE,
                         WACS_ULCORNER, WACS_URCORNER, WACS_LLCORNER,
                         WACS_LRCORNER) == OK);
      break;
    }
    default: {
      assert(wborder_set(ptr_, WACS_VLINE, WACS_VLINE, WACS_HLINE, WACS_HLINE,
                         nullptr, nullptr, nullptr, nullptr) == OK);
      break;
    }
    }
    break;
  }
  case (BorderStyle::kThick): {
    switch (style_.corner_style) {
    case (CornerStyle::kPlus): {
      assert(wborder_set(ptr_, WACS_T_VLINE, WACS_T_VLINE, WACS_T_HLINE,
                         WACS_T_HLINE, WACS_T_PLUS, WACS_T_PLUS, WACS_T_PLUS,
                         WACS_T_PLUS) == OK);
      break;
    }
    case (CornerStyle::kClosed): {
      assert(wborder_set(ptr_, WACS_T_VLINE, WACS_T_VLINE, WACS_T_HLINE,
                         WACS_T_HLINE, WACS_T_ULCORNER, WACS_T_URCORNER,
                         WACS_T_LLCORNER, WACS_T_LRCORNER) == OK);
      break;
    }
    default: {
      assert(wborder_set(ptr_, WACS_T_VLINE, WACS_T_VLINE, WACS_T_HLINE,
                         WACS_T_HLINE, nullptr, nullptr, nullptr,
                         nullptr) == OK);
      break;
    }
    }
    break;
  }
  case (BorderStyle::kDouble): {
    switch (style_.corner_style) {
    case (CornerStyle::kPlus): {
      assert(wborder_set(ptr_, WACS_D_VLINE, WACS_D_VLINE, WACS_D_HLINE,
                         WACS_D_HLINE, WACS_D_PLUS, WACS_D_PLUS, WACS_D_PLUS,
                         WACS_D_PLUS) == OK);
      break;
    }
    case (CornerStyle::kClosed): {
      assert(wborder_set(ptr_, WACS_D_VLINE, WACS_D_VLINE, WACS_D_HLINE,
                         WACS_D_HLINE, WACS_D_ULCORNER, WACS_D_URCORNER,
                         WACS_D_LLCORNER, WACS_D_LRCORNER) == OK);
      break;
    }
    default: {
      assert(wborder_set(ptr_, WACS_D_VLINE, WACS_D_VLINE, WACS_D_HLINE,
                         WACS_D_HLINE, nullptr, nullptr, nullptr,
                         nullptr) == OK);
      break;
    }
    }
    break;
  }
  default: {
    // none
  }
  }
}

} // namespace ui
} // namespace latis
