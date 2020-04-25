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

#include "src/ui/common.h"
#include "src/ui/window.h"

#include "absl/memory/memory.h"
#include <form.h>
#include <ncurses.h>

namespace latis {
namespace ui {

class Widget {
public:
  virtual ~Widget() = default;
  virtual void BubbleCh(int ch) = 0;
  virtual void BubbleEvent(const MEVENT &event) = 0;
};

// // RAII for forms. Form::~Form() is your friend.
// class Form {
// public:
//   Form(WINDOW *window_ptr);
//   ~Form();
//   void BubbleCh(int ch);
//
// private:
//   WINDOW *window_ptr_;
//   FORM *form_{nullptr};
//   FIELD *field_[1];
// };

class Textbox : public Widget {
public:
  Textbox(Dimensions dimensions, std::function<void(absl::string_view)> recv_cb,
          Opts opts);
  ~Textbox() override {}
  void Update(absl::string_view s);
  void Clear();
  void BubbleCh(int ch) override;
  void BubbleEvent(const MEVENT &event) override;

private:
  const Opts opts_;
  const std::function<void(absl::string_view)> recv_cb_;
  const std::unique_ptr<Window> window_;

  // std::unique_ptr<Form> form_{nullptr};
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_WIDGET_H_
