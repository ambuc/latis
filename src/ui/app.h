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
#include <ncurses.h>

namespace latis {
namespace ui {

class App {
public:
  App();
  ~App();

  std::shared_ptr<Textbox>
  AddTextbox(absl::string_view title, Dimensions dimensions,
             std::function<void(absl::string_view)> recv_cb);

  // If you don't care about the recv_cb.
  std::shared_ptr<Textbox> AddTextbox(absl::string_view title,
                                      Dimensions dimensions);

  std::shared_ptr<Widget> Get(absl::string_view title);

  template <typename T> //
  std::shared_ptr<T> Get(absl::string_view title) {
    if (auto ptr = Get(title); ptr == nullptr) {
      return nullptr;
    } else {
      return std::dynamic_pointer_cast<T>(ptr);
    }
  }

  void Remove(absl::string_view title);

private:
  absl::flat_hash_map<std::string, std::shared_ptr<Widget>> widgets_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_APP_H_
