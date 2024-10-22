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

#ifndef SRC_SSHEET_INTERFACE_H_
#define SRC_SSHEET_INTERFACE_H_

#include "proto/latis_msg.pb.h"
#include "src/xy.h"

#include "absl/time/time.h"
#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/status_macros.h"
#include "google/protobuf/stubs/statusor.h"

namespace latis {

using HasChangedCb = std::function<void(const Cell &)>;
using EditedTimeCb = std::function<void(absl::Time)>;

// SSheetInterface is the spreadsheet engine. It doesn't know anything about
// graphics or display or anything, it just has a few methods. Thread-safe.
class SSheetInterface {
public:
  virtual ::google::protobuf::util::StatusOr<Amount> Get(XY xy) const = 0;

  virtual ::google::protobuf::util::StatusOr<Amount>
  Set(XY xy, std::string_view input) = 0;

  virtual void Clear(XY xy) = 0;

  virtual ::google::protobuf::util::Status
  WriteTo(LatisMsg *latis_msg) const = 0;

  virtual void RegisterCallback(HasChangedCb has_changed_cb) = 0;
  virtual void RegisterEditedTimeCallback(EditedTimeCb edited_time_cb) = 0;

  virtual absl::optional<std::string> Title() const = 0;
  virtual void SetTitle(absl::string_view title) = 0;
  virtual absl::optional<std::string> Author() const = 0;
  virtual void SetAuthor(absl::string_view author) = 0;
  virtual absl::Time CreatedTime() const = 0;
  virtual absl::Time EditedTime() const = 0;
};

} // namespace latis

#endif // SRC_SSHEET_INTERFACE_H_
