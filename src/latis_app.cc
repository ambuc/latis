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
#include "src/latis_impl.h"

#include <ncurses.h>

namespace latis {

LatisApp::LatisApp(Options options)
    : options_(options), latis_obj_(absl::make_unique<Latis>()),
      app_(absl::make_unique<ui::App>()) {
  app_->AddTextbox("title", {3, 40, 0, 0},
                   [this](absl::string_view s) { latis_obj_->SetTitle(s); })
      ->Update(absl::StrFormat("Title: %s", latis_obj_->Title().value_or("")));

  app_->AddTextbox("author", {3, 40, 0, 39},
                   [this](absl::string_view s) { latis_obj_->SetAuthor(s); })
      ->Update(
          absl::StrFormat("Author: %s", latis_obj_->Author().value_or("")));

  app_->AddTextbox("date_created", {3, 40, 2, 0})
      ->Update(absl::StrFormat("Date Created: %s",
                               absl::FormatTime(latis_obj_->CreatedTime())));
  app_->AddTextbox("date_edited", {3, 40, 2, 39})
      ->Update(absl::StrFormat("Date Edited: %s",
                               absl::FormatTime(latis_obj_->EditedTime())));

  latis_obj_->RegisterEditedTimeCallback([this](absl::Time t) {
    app_->Get<latis::ui::Textbox>("date_edited")
        ->Update(absl::StrFormat("Date Edited: %s", absl::FormatTime(t)));
  });

  // Maybe instantiate debug textbox.
  if (options.debug) {
    int y, x;
    getmaxyx(stdscr, y, x);
    app_->AddTextbox("debug", {3, x, y - 3, 0})->Update("DEBUG_MODE_ENABLED");
  }
}

void LatisApp::Load(LatisMsg msg) {
  latis_obj_ = absl::make_unique<Latis>(msg);
}

void LatisApp::BubbleCh(int ch) { app_->BubbleCh(ch); }

void LatisApp::BubbleEvent(const MEVENT &event) { app_->BubbleEvent(event); }

} // namespace latis
