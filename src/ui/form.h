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
#ifndef SRC_UI_FORM_H_
#define SRC_UI_FORM_H_

#include "src/ui/window.h"

#include "absl/memory/memory.h"
#include <form.h>
#include <ncurses.h>

namespace latis {
namespace ui {

// Forms are hard.
// https://invisible-island.net/ncurses/ncurses-intro.html#form
//
// The general flow of control of a form program looks like this:
//   1. Create the form fields, using new_field().
//   2. Create the form using new_form().
//   3. Post the form using post_form().
//   4. Refresh the screen.
//   5. Process user requests via an input loop.
//   6. Unpost the form using unpost_form().
//   7. Free the form, using free_form().
//   8. Free the fields using free_field().

class Form {
public:
  Form(Window *window, Opts opts);
  ~Form();

private:
  Window *window_{nullptr};
  const Opts opts_;

  FORM *form_{nullptr};
  FIELD *fields_[2]{nullptr, nullptr};
};

} // namespace ui
} // namespace latis

#endif // SRC_UI_FORM_H_
