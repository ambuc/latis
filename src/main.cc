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

#include "proto/latis_msg.pb.h"
#include "src/latis_impl.h"
#include "src/ui/app.h"
#include "src/utils/io.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

ABSL_FLAG(std::string, textproto_input, "", "Path to input textproto");
ABSL_FLAG(bool, debug_mode, false,
          "If true, prints debug stuff at the bottom.");

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);

  latis::Latis latis_obj(
      latis::FromTextproto<LatisMsg>(absl::GetFlag(FLAGS_textproto_input))
          .ValueOrDie());

  latis::ui::App app;

  app.AddTextbox("title", {3, 40, 0, 0},
                 [&latis_obj](absl::string_view s) { latis_obj.SetTitle(s); })
      ->Update(absl::StrFormat("Title: %s", latis_obj.Title().value_or("")));

  app.AddTextbox("author", {3, 40, 0, 39},
                 [&latis_obj](absl::string_view s) { latis_obj.SetAuthor(s); })
      ->Update(absl::StrFormat("Author: %s", latis_obj.Author().value_or("")));

  app.AddTextbox("date_created", {3, 40, 2, 0})
      ->Update(absl::StrFormat("Date Created: %s",
                               absl::FormatTime(latis_obj.CreatedTime())));
  app.AddTextbox("date_edited", {3, 40, 2, 39})
      ->Update(absl::StrFormat("Date Edited: %s",
                               absl::FormatTime(latis_obj.EditedTime())));

  // When the latis_obj changes, change this too.
  latis_obj.RegisterEditedTimeCallback([&app](absl::Time t) {
    app.Get<latis::ui::Textbox>("date_edited")
        ->Update(absl::StrFormat("Date Edited: %s", absl::FormatTime(t)));
  });

  // Maybe instantiate debug textbox.
  if (absl::GetFlag(FLAGS_debug_mode)) {
    int y, x;
    getmaxyx(stdscr, y, x);
    app.AddTextbox("debug", {3, x, y - 3, 0})->Update("DEBUG_MODE_ENABLED");
  }

  // read-eval-print loop
  MEVENT event;
  while (true) {
    if (int ch = getch(); getmouse(&event) == OK) {
      app.BubbleEvent(event);
    } else if (ch == int('q')) {
      break; // and return
    } else {
      app.BubbleCh(ch);
    }
  }

  return 0;
}
