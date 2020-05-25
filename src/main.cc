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
ABSL_FLAG(std::string, input, "", "Input textproto");

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);

  std::unique_ptr<latis::LatisApp> latis_app;

  if (const auto path = absl::GetFlag(FLAGS_textproto_input); !path.empty()) {
    // If --textproto_input is set, read a file and load it in.
    auto msg = latis::FromTextproto<LatisMsg>(path).ValueOrDie();
    latis_app = absl::make_unique<latis::LatisApp>(msg);
  } else if (const auto input = absl::GetFlag(FLAGS_input); !input.empty()) {
    auto msg = latis::FromText<LatisMsg>(input).ValueOrDie();
    latis_app = absl::make_unique<latis::LatisApp>(msg);
  } else {
    latis_app = absl::make_unique<latis::LatisApp>();
  }

  latis_app->Run();

  return 0;
}
