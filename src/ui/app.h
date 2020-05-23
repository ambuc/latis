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

#include "absl/container/flat_hash_map.h"
#include <memory>

namespace latis {
namespace ui {

class App {
public:
  using ResizeCb = std::function<void(void)>;

  explicit App();
  ~App();

  template <typename T, typename... Args> //
  T *Add(absl::string_view title, Args... args) {
    widgets_[std::string(title)] = std::make_unique<T>(args...);
    ui::Debug(absl::StrFormat("%d widgets now.", widgets_.size()));
    return Get<T>(title);
  }

  // Get (or create) widget of a particular name and type.
  template <typename T> //
  T *Get(absl::string_view title) {
    return dynamic_cast<T *>(widgets_[std::string(title)].get());
  }

  // Remove widget of any kind by a given name.
  void Remove(absl::string_view title);

  void RemoveAllWidgets();

  // Run the app.
  void Run();

  void SetActive(Widget *w) { active_ = w; }

  // Registers a callback to be invoked when the window resized.
  void RegisterResizeCallback(ResizeCb cb);

private:
  // Will never be nullptr.
  Widget *active_;

  absl::flat_hash_map<std::string, std::unique_ptr<Widget>> widgets_;

  absl::optional<ResizeCb> resize_cb_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_APP_H_
