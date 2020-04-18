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
#ifndef SRC_UI_APP_H_
#define SRC_UI_APP_H_

#include "absl/container/flat_hash_map.h"
#include <ncurses.h>

namespace latis {
namespace ui {

class Window {
public:
  struct Opts {
    bool border = false;
    bool dimensions = false;
  };
  Window(int nlines, int ncols, int begin_y, int begin_x, Opts opts);
  Window(int nlines, int ncols, int begin_y, int begin_x)
      : Window(nlines, ncols, begin_y, begin_x, Opts()){};
  ~Window();
  operator WINDOW *() { return ptr_; }
  void Put(absl::string_view input);

private:
  const int nlines_, ncols_, begin_y_, begin_x_;
  const Opts opts_;
  WINDOW *ptr_;
};

class App {
public:
  struct Opts {
    bool enable_window_dimensions = false;
  };

  App() : App(Opts()) {}
  explicit App(Opts opts);
  ~App();

  void InsertWindow(std::string title, std::unique_ptr<Window> w);
  Window *GetWindow(std::string title);

private:
  Opts opts_;
  absl::flat_hash_map<std::string, std::unique_ptr<Window>> windows_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_APP_H_
