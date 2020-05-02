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
#include <signal.h>

namespace latis {

LatisApp::LatisApp(LatisMsg msg)
    : ssheet_(absl::make_unique<SSheet>(msg)),
      app_(absl::make_unique<ui::App>()) {

  int y, x;
  getmaxyx(stdscr, y, x);

  ui::LayoutEngine layout_engine(y, x);

  const auto default_dims = ui::Dimensions{5, 5, 0, 0};

  auto dims_title =
      layout_engine.Place(/*h=*/3, /*w=*/x / 4).value_or(default_dims);
  auto dims_author =
      layout_engine.Place(/*h=*/3, /*w=*/x / 4).value_or(default_dims);
  auto dims_created =
      layout_engine.Place(/*h=*/3, /*w=*/x / 4).value_or(default_dims);
  auto dims_edited =
      layout_engine.Place(/*h=*/3, /*w=*/x / 4).value_or(default_dims);

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

  layout_engine.Place(/*h=*/1, /*w=*/x); // newline

  auto dims_gridbox =
      layout_engine.Place(/*h=*/20, /*w=*/x).value_or(default_dims);
  // TODO(ambuc): Refuse xy placements greater than num_lines / num_cols
  auto gridbox_ptr = app_->Add<ui::Gridbox>("gridbox", dims_gridbox,
                                            /*num_lines=*/5, /*num_cols=*/5);
  assert(gridbox_ptr != nullptr);

  for (int y = 0; y <= ssheet_->Height(); ++y) {
    for (int x = 0; x <= ssheet_->Width(); ++x) {
      auto xy = XY(x, y);
      if (const auto amt = ssheet_->Get(xy); amt.ok()) {
        gridbox_ptr->Add<ui::Textbox>(y, x)
            ->WithCb([this, xy](absl::string_view s) { ssheet_->Set(xy, s); })
            ->Update(PrintAmount(amt.ValueOrDie()));
      }
    }
  }
}

void LatisApp::Run() { app_->Run(); }

} // namespace latis
