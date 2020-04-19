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
#include "src/latis_app.h"
#include "src/ui/app.h"
#include "src/utils/io.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

ABSL_FLAG(std::string, textproto_input, "", "Path to input textproto");
ABSL_FLAG(bool, debug_mode, false,
          "If true, prints debug stuff at the bottom.");

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);

  latis::LatisApp latis_app({
      .debug = absl::GetFlag(FLAGS_debug_mode),
  });

  if (const auto path = absl::GetFlag(FLAGS_textproto_input); !path.empty()) {
    latis_app.Load(latis::FromTextproto<LatisMsg>(path).ValueOrDie());
  }

  // read-eval-print loop
  MEVENT event;
  while (true) {
    if (int ch = getch(); getmouse(&event) == OK) {
      latis_app.BubbleEvent(event);
    } else if (ch == int('q')) {
      break; // and return
    } else {
      latis_app.BubbleCh(ch);
    }
  }

  return 0;
}
