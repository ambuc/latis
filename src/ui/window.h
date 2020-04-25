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

class Window {
public:
  Window(Dimensions dimensions, Opts opts);
  Window(Dimensions dimensions) : Window(dimensions, Opts()){};
  ~Window();

  void Print(int y, int x, absl::string_view s);
  void Refresh();
  void Clear();

  bool Contains(int y, int x) const;
  int Width() const;
  int Height() const;

  WINDOW *operator*() const { return ptr_; }

private:
  void PrintPermanentComponents();
  const Dimensions dimensions_;
  const Opts opts_;
  WINDOW *ptr_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_WINDOW_H_
