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
#include "src/ui/layout_engine.h"

#include <ncurses.h>

namespace latis {

LatisApp::LatisApp(ui::Opts opts, LatisMsg msg)
    : opts_(opts), ssheet_(absl::make_unique<SSheet>(msg)),
      app_(absl::make_unique<ui::App>(opts_)) {

  int y, x;
  getmaxyx(stdscr, y, x);

  ui::LayoutEngine e(y, x);

  const auto default_dims = ui::Dimensions{3, 3, 0, 0};

  auto dims_title = e.PlaceTL(/*h=*/3, /*w=*/x / 4).value_or(default_dims);
  auto dims_author = e.PlaceTL(/*h=*/3, /*w=*/x / 4).value_or(default_dims);
  auto dims_created = e.PlaceTL(/*h=*/3, /*w=*/x / 4).value_or(default_dims);
  auto dims_edited = e.PlaceTL(/*h=*/3, /*w=*/x / 4).value_or(default_dims);

  auto dims_debug = e.PlaceTL(/*h=*/3, /*w=*/x / 2).value_or(default_dims);
  auto dims_fc = e.PlaceTL(/*h=*/3, /*w=*/x / 2).value_or(default_dims);

  app_->Add<ui::Textbox>("title", dims_title)
      ->WithTemplate(
          [](std::string s) { return absl::StrFormat("Title: %s", s); })
      ->WithCb([this](absl::string_view s) { ssheet_->SetTitle(s); })
      ->Update(ssheet_->Title().value_or("n/a"));

  app_->Add<ui::Textbox>("author", dims_author)
      ->WithTemplate(
          [](std::string s) { return absl::StrFormat("Author: %s", s); })
      ->WithCb([this](absl::string_view s) { ssheet_->SetAuthor(s); })
      ->Update(ssheet_->Author().value_or("no author"));

  app_->Add<ui::Textbox>("date_created", dims_created)
      ->Update(absl::StrFormat("Date Created: %s",
                               absl::FormatTime(ssheet_->CreatedTime())));

  app_->Add<ui::Textbox>("date_edited", dims_edited)
      ->Update(absl::StrFormat("Date Edited: %s",
                               absl::FormatTime(ssheet_->EditedTime())));

  ssheet_->RegisterEditedTimeCallback([this](absl::Time t) {
    app_->Get<ui::Textbox>("date_edited")
        ->Update(absl::StrFormat("Date Edited: %s", absl::FormatTime(t)));
  });

  // Maybe instantiate debug textbox.
  if (opts_.show_debug_textbox) {
    debug_tbx_ = app_->Add<ui::Textbox>("debug_textbox", dims_debug);
    debug_tbx_->Update("DEBUG_MODE_ENABLED");

    fc_tbx_ = app_->Add<ui::Textbox>("frame_count", dims_fc);
    fc_tbx_->Update("FRAME_COUNT");
  }
}

void LatisApp::Run() {
  MEVENT event;
  while (true) {

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
      fc_tbx_->Update(std::to_string(frame_));
      debug_tbx_->Update((has_event)
                             ? absl::StrFormat("Mousepress: %d,%d,%d,%d", ch,
                                               event.bstate, event.x, event.y)
                             : absl::StrFormat("Keypress: %d", ch));
    }
    frame_++;
    frame_ %= 100;
  }
}

} // namespace latis
