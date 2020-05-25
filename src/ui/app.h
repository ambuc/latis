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
#ifndef SRC_UI_APP_H_
#define SRC_UI_APP_H_

#include "src/ui/common.h"
#include "src/ui/widget.h"
#include "src/ui/window.h"

#include "absl/container/flat_hash_set.h"
#include <memory>

namespace latis {
namespace ui {

class App {
public:
  using ResizeCb = std::function<void(void)>;

  explicit App();
  ~App();

  template <typename T, typename... Args> //
  std::shared_ptr<T> Add(Args... args) {
    auto p = std::make_shared<T>(args...);
    widgets_.insert(p);
    ui::Debug(absl::StrFormat("%d widgets now.", widgets_.size()));
    active_ = std::make_unique<ActiveWidget>(p);
    return p;
  }

  void RemoveAllWidgets();

  // Run the app.
  void Run();

  // Registers a callback to be invoked when the window resized.
  void RegisterResizeCallback(ResizeCb cb);

private:
  // Will never be nullptr.
  std::unique_ptr<ActiveWidget> active_;

  absl::flat_hash_set<std::shared_ptr<Widget>> widgets_;

  absl::optional<ResizeCb> resize_cb_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_APP_H_
