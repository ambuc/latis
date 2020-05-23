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

#include "src/integration_tests/integration_test_base.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"
#include "absl/time/clock.h"
#include "proto/latis_msg.pb.h"

namespace latis {
namespace test {

namespace {

// https://www.jeremymorgan.com/tutorials/c-programming/how-to-capture-the-output-of-a-linux-command-in-c/
std::string GetStdoutFromCommand(std::string cmd, int max_buffer = 256) {
  std::string data;
  FILE *stream;
  char buffer[max_buffer];
  cmd.append(" 2>&1");

  stream = popen(cmd.c_str(), "r");
  if (stream) {
    while (!feof(stream))
      if (fgets(buffer, max_buffer, stream) != NULL)
        data.append(buffer);
    pclose(stream);
  }
  return data;
}

} // namespace

void IntegrationTestBase::SetUp() {
  std::system(
      absl::StrFormat("tmux new-session -d -s %s", tmux_session_name_).c_str());
  std::system(
      absl::StrFormat("tmux resize-pane -x 256 -t %s:1", tmux_session_name_)
          .c_str());
}
// Tear down TMUX session
void IntegrationTestBase::TearDown() {
  std::system(
      absl::StrFormat("tmux kill-session -t %s", tmux_session_name_).c_str());
}

// Send a command and wait a beat for the inputs to be processed
void IntegrationTestBase::Send(const std::string &cmd) {
  absl::SleepFor(absl::Milliseconds(1000));
  std::system(absl::StrFormat("tmux send-keys -t %s \"%s\" ENTER",
                              tmux_session_name_, cmd)
                  .c_str());
  absl::SleepFor(absl::Milliseconds(1000));
}

// Capture and return the output as a string
std::string IntegrationTestBase::Dump() {
  return GetStdoutFromCommand(
      absl::StrFormat("tmux capture-pane -p -t %s:1", tmux_session_name_));
}

} // namespace test
} // namespace latis
