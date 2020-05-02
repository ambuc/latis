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
#ifndef SRC_UI_WIDGET_H_
#define SRC_UI_WIDGET_H_

#include "src/ui/window.h"

namespace latis {
namespace ui {

class Widget {
public:
  Widget(std::unique_ptr<Window> window);
  virtual ~Widget() = default;

  void Clear();

  // Returns true if this widget consumed the event.
  virtual bool Process(int ch, const MEVENT &event, bool is_mouse) = 0;

protected:
  std::unique_ptr<Window> window_;
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_WIDGET_H_
