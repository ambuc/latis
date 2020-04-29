/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SRC_UI_WINDOW_H_
#define SRC_UI_WINDOW_H_

#include "absl/strings/string_view.h"
#include "src/ui/common.h"
#include <ncurses.h>

namespace latis {
namespace ui {

// Thin wrapper around ncurses' WINDOW struct. RAII-style, ~Window() handles
// necessary deletion and cleanup.
class Window {
public:
  Window(Dimensions dimensions, Opts opts, BorderStyle border_style,
         WINDOW *window);
  // Default WINDOW
  Window(Dimensions dimensions, Opts opts)
      : Window(dimensions, opts, BorderStyle::kThin,
               newwin(dimensions.nlines, dimensions.ncols, dimensions.begin_y,
                      dimensions.begin_x)) {}
  // Default opts and WINDOW.
  Window(Dimensions dimensions) : Window(dimensions, Opts()){};
  ~Window();

  // Prints the string |s| to coordinates (x,y) within the window.
  // Refreshes the window.
  void Print(int y, int x, absl::string_view s);

  // Refreshes the window. Useful for outside methods which take this window as
  // their canvas.
  void Refresh();

  // Clears the contents of the window, and refreshes the view.
  void Clear();

  // Gets the underlying Dimensions struct. Useful for querying .Contains(),
  // .Width(), .Height(), etc.
  Dimensions GetDimensions() const;
  Opts GetOpts() const;

  // Useful for accessing the underlying WINDOW ptr for use in registering
  // forms, etc.
  WINDOW *operator*() const { return ptr_; }

private:
  void PrintPermanentComponents();
  const Dimensions dimensions_;
  const Opts opts_;
  const BorderStyle border_style_;
  WINDOW *ptr_;
};

// // TODO(ambuc): Add this Pad class and really think about its API.
// // https://invisible-island.net/ncurses/man/curs_pad.3x.html
//
// class Pad : public Window {
// public:
//   Pad(Dimensions dimensions, Opts opts)
//       : Window(dimensions, opts, newpad(dimensions.nlines, dimensions.ncols))
//       {}
//   Pad(Dimensions dimensions) : Pad(dimensions, Opts()){};
//   ~Pad();
//   void Refresh() override { prefresh(ptr_, sm); }
// };

} // namespace ui
} // namespace latis

#endif // SRC_UI_WINDOW_H_
