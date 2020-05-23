
// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/ui/formwidget.h"

#include "absl/strings/ascii.h"

namespace latis {
namespace ui {

FormWidget::FormWidget(std::unique_ptr<Window> window,
                       absl::string_view placeholder)
    : Widget(std::move(window)) {
  Debug(absl::StrFormat("FormWidget::FormWidget(%p,%s)", window.get(),
                        placeholder));
  // The general flow of control of a form program looks like this:
  //   1. Create the form fields, using new_field().

  fields_[0] =
      new_field(/*height=*/1, /*width=*/window_->GetDimensions().Width() - 4,
                /*toprow=*/1,
                /*leftcol=*/2, /*offscreen=*/0, /*numbuf=*/0);
  assert(errno == E_OK);
  assert(fields_[0] != nullptr);

  assert(E_OK == set_field_back(fields_[0], A_UNDERLINE)); // Print a line
  assert(E_OK == field_opts_off(fields_[0], O_AUTOSKIP | O_STATIC));

  set_field_buffer(fields_[0], 0, std::string(placeholder).data());

  // bold and colorful
  const int input_colors = 1;
  start_color();
  init_pair(input_colors, COLOR_CYAN, COLOR_BLACK);
  assert(E_OK == set_field_fore(fields_[0], A_BOLD | COLOR_PAIR(input_colors)));

  //   2. Create the form using new_form().
  form_ = new_form(fields_);
  set_form_fields(form_, fields_);
  assert(form_fields(form_) == fields_);
  assert(field_count(form_) == 1);
  assert(form_ != nullptr);
  assert(errno == E_OK);

  int rows;
  int cols;
  scale_form(form_, &rows, &cols);

  assert(E_OK == set_form_sub(form_, **window_));

  //   3. Post the form using post_form().
  assert(E_OK == post_form(form_));

  set_current_field(form_, fields_[0]);
}

int FormWidget::Process(int ch) {
  Debug(absl::StrFormat("FormWidget::Process(%c)", ch));

  switch (ch) {
  case KEY_ENTER:
  case 10:
    form_driver(form_, REQ_NEXT_FIELD);
    break;
  case KEY_BACKSPACE:
  case 127:
    form_driver(form_, REQ_DEL_PREV);
    break;
  case KEY_LEFT:
    form_driver(form_, REQ_PREV_CHAR);
    break;
  case KEY_RIGHT:
    form_driver(form_, REQ_NEXT_CHAR);
    break;
  default:
    form_driver(form_, ch);
    break;
  }
  window_->Refresh();
  return 0;
}

FormWidget::~FormWidget() {
  Debug("FormWidget::~FormWidget");

  // We don't want to clean up window_ via unpost_form or free_field, since
  // window_ already gets cleaned up by Window::~Window().
}

std::string FormWidget::Extract() {
  Debug("FormWidget::Extract");

  assert(field_status(fields_[0]) == true);
  return std::string(
      absl::StripTrailingAsciiWhitespace(field_buffer(fields_[0], 0)));
}

} // namespace ui
} // namespace latis
