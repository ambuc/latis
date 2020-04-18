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

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include <fcntl.h>
#include <fstream>
#include <iostream>

ABSL_FLAG(std::string, textproto_input, "", "Path to input textproto");

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);

  const std::string path = absl::GetFlag(FLAGS_textproto_input);
  if (path.empty()) {
    std::cout << "FLAGS_textproto_input was empty." << std::endl;
    return 1;
  }

  LatisMsg parsed;
  int fd = open(path.c_str(), O_RDONLY);
  google::protobuf::io::FileInputStream fstream(fd);
  google::protobuf::TextFormat::Parse(&fstream, &parsed);
  close(fd);

  latis::Latis latis_obj(parsed);

  latis::ui::App app;

  const std::string kTitle = "title";
  app.InsertWindow(kTitle,
                   absl::make_unique<latis::ui::Window>(
                       3, 40, 0, 0, latis::ui::Window::Opts{.border = true}));
  app.GetWindow(kTitle)->Put(
      absl::StrFormat("Title: %s", latis_obj.Title().value_or("")));

  const std::string kAuthor = "author";
  app.InsertWindow(kAuthor,
                   absl::make_unique<latis::ui::Window>(
                       3, 40, 0, 40, latis::ui::Window::Opts{.border = true}));
  app.GetWindow(kAuthor)->Put(
      absl::StrFormat("Author: %s", latis_obj.Author().value_or("")));

  absl::SleepFor(absl::Minutes(1));

  return 0;
}
