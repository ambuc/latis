/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SRC_INTEGRATION_TESTS_INTEGRATION_TEST_BASE_H_
#define SRC_INTEGRATION_TESTS_INTEGRATION_TEST_BASE_H_

#include "proto/latis_msg.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace test {

class IntegrationTestBase : public ::testing::Test {
public:
  // Create TMUX session
  void SetUp() override;

  // Tear down TMUX session
  void TearDown() override;

  // Send a command and wait a beat for the inputs to be processed
  void Send(const std::string &cmd);

  // Given a LatisMsg proto, sends it via `latis --input="%s"`.
  void SendLatisMsg(const LatisMsg &msg);

  // Capture and return the output as a string
  std::string Dump();

  const std::string tmux_session_name_ = "latis_integration_test_session";
};

} // namespace test
} // namespace latis

#endif // SRC_INTEGRATION_TESTS_INTEGRATION_TEST_BASE_H_
