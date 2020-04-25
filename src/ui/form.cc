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

#include "src/ui/form.h"

#include "absl/strings/str_format.h"

#include <form.h>
#include <iostream>
#include <ncurses.h>

namespace latis {
namespace ui {

Form::Form(Window *window, Opts opts) : window_(window), opts_(opts) {
  if (opts_.show_debug_textbox) {
    window_->Print(0, 0, "starting form");
    window_->Refresh();
  }

  fields_[0] = new_field(/*height=*/1, /*width=*/window_->Width(), /*toprow=*/0,
                         /*leftcol=*/0, /*offscreen=*/0, /*numbuf=*/0);
  assert(errno == E_OK);
  assert(fields_[0] != nullptr);

  if (opts_.show_debug_textbox) {
    window_->Print(0, 0, "created all fields");
    window_->Refresh();
  }

  assert(E_OK == set_field_back(fields_[0], A_UNDERLINE)); // Print a line
  assert(E_OK ==
         field_opts_off(fields_[0], O_AUTOSKIP)); // Don't skip when full

  if (opts_.show_debug_textbox) {
    window_->Print(0, 0, "set all field options");
    window_->Refresh();
  }

  form_ = new_form(fields_);
  set_form_fields(form_, fields_);
  assert(form_fields(form_) == fields_);
  assert(field_count(form_) == 1);
  assert(form_ != nullptr);

  assert(errno != E_BAD_ARGUMENT);
  assert(errno != E_BAD_STATE);
  assert(errno != E_NOT_POSTED);
  assert(errno != E_NOT_CONNECTED);
  assert(errno != E_CONNECTED);
  assert(errno != E_NO_ROOM);
  assert(errno != E_POSTED);
  assert(errno != E_SYSTEM_ERROR);
  assert(errno == E_OK);

  if (opts_.show_debug_textbox) {
    window_->Print(0, 0, absl::StrFormat("did new_form() for form: %p", form_));
    window_->Refresh();
  }

  // assert(E_OK == set_form_win(form_, **window_)); // Link form to window.
  assert(E_OK == set_form_sub(form_, **window_)); // Link form to window.

  if (opts_.show_debug_textbox) {
    window_->Print(0, 0, "set form sub.");
    window_->Refresh();
  }

  assert(E_OK == post_form(form_));
  set_current_field(form_, fields_[0]);
  noecho();

  if (opts_.show_debug_textbox) {
    window_->Print(0, 0, "posted form");
    window_->Refresh();
  }

  window_->Refresh();
}

Form::~Form() {
  assert(E_OK == unpost_form(form_));

  assert(E_OK == free_form(form_));
  assert(E_OK == free_field(fields_[0]));

  window_->Refresh();
}

} // namespace ui
} // namespace latis
