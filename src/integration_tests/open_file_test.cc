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

#include "absl/strings/str_format.h"
#include "absl/strings/strip.h"
#include "absl/time/clock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstdlib>

namespace latis {
namespace {

using ::testing::Eq;

// https://www.jeremymorgan.com/tutorials/c-programming/how-to-capture-the-output-of-a-linux-command-in-c/
std::string GetStdoutFromCommand(std::string cmd) {
  std::string data;
  FILE *stream;
  const int max_buffer = 256;
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

class IntegrationTestBase : public ::testing::Test {
public:
  // Create TMUX session
  void SetUp() override {
    std::system(absl::StrFormat("tmux new-session -d -s %s", tmux_session_name_)
                    .c_str());
  }
  // Tear down TMUX session
  void TearDown() override {
    std::system(
        absl::StrFormat("tmux kill-session -t %s", tmux_session_name_).c_str());
  }
  // Send a command and wait a beat for the inputs to be processed
  void Send(const std::string &cmd) {
    std::system(absl::StrFormat("tmux send-keys -t %s \"%s\" ENTER",
                                tmux_session_name_, cmd)
                    .c_str());
    absl::SleepFor(absl::Milliseconds(500));
  }
  // Capture and return the output as a string
  std::string Dump() {
    return GetStdoutFromCommand(
        absl::StrFormat("tmux capture-pane -p -t %s:1", tmux_session_name_));
  }

  const std::string tmux_session_name_ = "latis_integration_test_session";
};

TEST_F(IntegrationTestBase, TryGetStdoutFromCommand) {
  Send("pwd");

  EXPECT_THAT(Dump(), ::testing::HasSubstr("/.cache/bazel/_bazel_"));
}

//
} // namespace
} // namespace latis
