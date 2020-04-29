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

  latis::ui::Opts opts{
      .show_borders = absl::GetFlag(FLAGS_debug_mode),
      .show_debug_textbox = absl::GetFlag(FLAGS_debug_mode),
      .show_frame_count = absl::GetFlag(FLAGS_debug_mode),
  };

  std::unique_ptr<latis::LatisApp> latis_app;

  // If --textproto_input is set, read a file and load it in.
  if (const auto path = absl::GetFlag(FLAGS_textproto_input); !path.empty()) {
    auto msg = latis::FromTextproto<LatisMsg>(path).ValueOrDie();
    latis_app = absl::make_unique<latis::LatisApp>(opts, msg);
  } else {
    latis_app = absl::make_unique<latis::LatisApp>(opts);
  }

  latis_app->Run();

  return 0;
}
