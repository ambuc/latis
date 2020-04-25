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
#include "src/ui/common.h"

namespace latis {

class LatisApp {
public:
  LatisApp() : LatisApp(ui::Opts()) {}
  explicit LatisApp(ui::Opts opts) : LatisApp(opts, LatisMsg()) {}
  LatisApp(ui::Opts opts, LatisMsg msg);
  void Run();

private:
  void WireUp();

  const ui::Opts opts_;
  std::unique_ptr<SSheet> ssheet_;
  std::unique_ptr<ui::App> app_;

  int frame_{0};

  // if .show_debug_textbox == false, these are all nullptr.
  std::shared_ptr<ui::Textbox> debug_tbx_;
  std::shared_ptr<ui::Textbox> fc_tbx_;
};

} // namespace latis

#endif //  SRC_LATIS_APP_H_
