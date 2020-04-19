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
#ifndef SRC_LATIS_APP_H_
#define SRC_LATIS_APP_H_

#include "proto/latis_msg.pb.h"
#include "src/ssheet_impl.h"
#include "src/ui/app.h"

namespace latis {

class LatisApp {
public:
  struct Options {
    bool debug = false;
  };

  LatisApp() : LatisApp(Options()) {}
  explicit LatisApp(Options options);
  void Load(LatisMsg msg);

  void BubbleCh(int ch);
  void BubbleEvent(const MEVENT &event);

private:
  const Options options_;
  std::unique_ptr<SSheet> ssheet_;
  std::unique_ptr<ui::App> app_;
};

} // namespace latis

#endif //  SRC_LATIS_APP_H_
