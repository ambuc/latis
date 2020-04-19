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

#include "src/latis_app.h"

#include "proto/latis_msg.pb.h"
#include "src/ssheet_impl.h"

#include <ncurses.h>

namespace latis {

LatisApp::LatisApp(ui::Opts opts)
    : opts_(opts), ssheet_(absl::make_unique<SSheet>()),
      app_(absl::make_unique<ui::App>()) {
  WireUp();
}

void LatisApp::Load(LatisMsg msg) { ssheet_ = absl::make_unique<SSheet>(msg); }

void LatisApp::ReadEvalPrintLoop() {
  MEVENT event;
  int count = 0;
  while (true) {
    count++;

    const int ch = getch();

    if (ch == ERR) {
      continue;
    } else if (ch == int('q')) {
      break;
    }

    bool has_event = (getmouse(&event) == OK);

    if (has_event) {
      app_->BubbleEvent(event);
    } else {
      app_->BubbleCh(ch);
    }

    if (opts_.show_debug_textbox) {
      fc_tbx_->Update(std::to_string(count));
      if (has_event) {
        debug_tbx_->Update(absl::StrFormat("Mousepress: %d,%d,%d,%d", ch,
                                           event.bstate, event.x, event.y));
      } else {
        debug_tbx_->Update(absl::StrFormat("Keypress: %d", ch));
      }
    }
  }
}

void LatisApp::WireUp() {
  app_->AddTextbox(
          "title", {3, 40, 0, 0},
          [this](absl::string_view s) { ssheet_->SetTitle(s); }, opts_)
      ->Update(absl::StrFormat("Title: %s", ssheet_->Title().value_or("n/a")));

  app_->AddTextbox(
          "author", {3, 40, 0, 39},
          [this](absl::string_view s) { ssheet_->SetAuthor(s); }, opts_)
      ->Update(
          absl::StrFormat("Author: %s", ssheet_->Author().value_or("n/a")));

  app_->AddTextbox(
          "date_created", {3, 40, 2, 0}, [](absl::string_view) {}, opts_)
      ->Update(absl::StrFormat("Date Created: %s",
                               absl::FormatTime(ssheet_->CreatedTime())));
  app_->AddTextbox(
          "date_edited", {3, 40, 2, 39}, [](absl::string_view) {}, opts_)
      ->Update(absl::StrFormat("Date Edited: %s",
                               absl::FormatTime(ssheet_->EditedTime())));

  ssheet_->RegisterEditedTimeCallback([this](absl::Time t) {
    app_->Get<latis::ui::Textbox>("date_edited")
        ->Update(absl::StrFormat("Date Edited: %s", absl::FormatTime(t)));
  });

  // Maybe instantiate debug textbox.
  if (opts_.show_debug_textbox) {
    int y, x;
    getmaxyx(stdscr, y, x);
    debug_tbx_ = app_->AddTextbox(
        "debug_textbox", {3, x, y - 3, 0}, [](absl::string_view) {}, opts_);
    debug_tbx_->Update("DEBUG_MODE_ENABLED");
    fc_tbx_ = app_->AddTextbox(
        "frame_count", {3, x, y - 5, 0}, [](absl::string_view) {}, opts_);
    fc_tbx_->Update("FRAME_COUNT");
  }
}

} // namespace latis
