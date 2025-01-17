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

#ifndef SRC_SSHEET_IMPL_H_
#define SRC_SSHEET_IMPL_H_

#include "src/ssheet_interface.h"

#include "proto/latis_msg.pb.h"
#include "src/display_utils.h"
#include "src/formula/common.h"
#include "src/formula/formula.h"
#include "src/graph/graph.h"
#include "src/xy.h"

#include "absl/base/thread_annotations.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"

namespace latis {

class SSheet : public SSheetInterface {
public:
  // Create new.
  SSheet();

  // Create from sheet.
  SSheet(const LatisMsg &sheet);

  ::google::protobuf::util::StatusOr<Amount> Get(XY xy) const override;

  ::google::protobuf::util::StatusOr<Amount>
  Set(XY xy, std::string_view input) override;

  void Clear(XY xy) override;

  ::google::protobuf::util::Status WriteTo(LatisMsg *latis_msg) const override;

  // NB: This only returns out-of-bound updates, i.e. cells _other_ than the
  // cell just set.
  void RegisterCallback(HasChangedCb has_changed_cb) override {
    has_changed_cb_ = has_changed_cb;
  }
  void RegisterEditedTimeCallback(EditedTimeCb edited_time_cb) override {
    edited_time_cb_ = edited_time_cb;
  }

  int Height() {
    // TODO cache this
    int height = 0;
    for (const auto &[xy, _] : cells_) {
      height = std::max(height, xy.Y());
    }
    return height;
  }

  int Width() {
    // TODO cache this
    int width = 0;
    for (const auto &[xy, _] : cells_) {
      width = std::max(width, xy.X());
    }
    return width;
  }

  absl::optional<std::string> Title() const override { return title_; }
  void SetTitle(absl::string_view title) override {
    UpdateEditTime();
    title_ = title;
  }
  absl::optional<std::string> Author() const override { return author_; }
  void SetAuthor(absl::string_view author) override {
    UpdateEditTime();
    author_ = author;
  }
  absl::Time CreatedTime() const override { return created_time_; }
  absl::Time EditedTime() const override { return edited_time_; }

private:
  void Update(XY xy);
  void UpdateEditTime();

  mutable absl::Mutex mu_;

  absl::flat_hash_map<XY, Cell> cells_ ABSL_GUARDED_BY(mu_);
  graph::Graph<XY> graph_;

  absl::optional<HasChangedCb> has_changed_cb_;
  absl::optional<EditedTimeCb> edited_time_cb_;

  // Metadata
  absl::optional<std::string> title_{std::nullopt};
  absl::optional<std::string> author_{std::nullopt};
  const absl::Time created_time_{absl::Now()};
  absl::Time edited_time_{absl::Now()};
};

} // namespace latis

#endif // SRC_SSHEET_IMPL_H_
