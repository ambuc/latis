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

#include <form.h>
#include <iostream>
#include <ncurses.h>

namespace latis {
namespace ui {

Window::Window(int nlines, int ncols, int begin_y, int begin_x, Opts opts)
    : nlines_(nlines), ncols_(ncols), begin_y_(begin_y), begin_x_(begin_x),
      opts_(opts), ptr_(newwin(nlines_, ncols_, begin_y_, begin_x_)) {
  if (opts_.border) {
    wborder(ptr_, '|', '|', '-', '-', '+', '+', '+', '+');
  }
  if (opts_.dimensions) {
    std::string dimensions = absl::StrFormat("%dx%d", nlines_, ncols_);
    mvwprintw(ptr_, 0, 0, dimensions.c_str());
  }
  wrefresh(ptr_);
}

void Window::Put(absl::string_view input) {
  mvwprintw(ptr_, 1, 2, std::string(input).c_str());
  wrefresh(ptr_);
}

Window::~Window() {
  delwin(ptr_);
  refresh();
}

App::App() {
  setlocale(LC_ALL, "");

  initscr();
  cbreak();
  noecho();
  clear();

  refresh();
}

App::~App() { clear(); }

void App::InsertWindow(std::string title, std::unique_ptr<Window> w) {
  windows_[title] = std::move(w);
}

Window *App::GetWindow(std::string title) {
  auto it = windows_.find(title);
  if (it == windows_.end()) {
    return nullptr;
  }
  return it->second.get();
}

} // namespace ui
} // namespace latis
