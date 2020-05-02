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
  explicit App();
  ~App();

  template <typename T, typename... Args> //
  std::shared_ptr<T> Add(absl::string_view title, Args... args) {
    auto widget = std::make_shared<T>(args...);
    widgets_[title] = widget;
    return widget;
  }

  // Get (or create) widget of a particular name and type.
  template <typename T> //
  std::shared_ptr<T> Get(absl::string_view title) {
    return std::dynamic_pointer_cast<T>(widgets_[title]);
  }

  // Remove widget of any kind by a given name.
  void Remove(absl::string_view title);

  // Run the app.
  void Run();

private:
  absl::flat_hash_map<std::string, std::shared_ptr<Widget>> widgets_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_APP_H_
