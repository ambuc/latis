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

#ifndef SRC_LATIS_IMPL_H_
#define SRC_LATIS_IMPL_H_

#include "src/latis_interface.h"

#include "proto/latis_msg.pb.h"
#include "src/display_utils.h"
#include "src/formula/common.h"
#include "src/formula/formula.h"
#include "src/xy.h"

#include "absl/base/thread_annotations.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"

namespace latis {

using UpdatedCb = std::function<void(const Cell &)>;

class Latis : public LatisInterface {
public:
  // Create new.
  Latis();

  // Create from sheet.
  Latis(const LatisMsg &sheet);

  ::google::protobuf::util::StatusOr<Amount> Get(XY xy) override;

  ::google::protobuf::util::Status Set(XY xy, std::string_view input) override;

  ::google::protobuf::util::Status WriteTo(LatisMsg *latis_msg) const override;

  // NB: This only returns out-of-bound updates, i.e. cells _other_ than the
  // cell just set.
  void RegisterUpdateCallback(UpdatedCb cell_updated_cb) {
    updated_callbacks_.push_back(cell_updated_cb);
  }

  absl::optional<std::string> Title() const override { return title_; }
  void SetTitle(std::string title) override {
    UpdateEditTime();
    title_ = title;
  }
  absl::optional<std::string> Author() const override { return author_; }
  void SetAuthor(std::string author) override {
    UpdateEditTime();
    author_ = author;
  }
  absl::Time CreatedTime() const override { return created_time_; }
  absl::Time EditedTime() const override { return edited_time_; }

private:
  void UpdateChildrenOf(XY xy);
  void Update(XY xy);
  void UpdateEditTime();

  // void PrintP2C();
  // void PrintC2P();

  mutable absl::Mutex mu_;

  absl::flat_hash_map<XY, Cell> cells_ ABSL_GUARDED_BY(mu_);

  // If                       B   C
  //                           \ /
  //   A := Fn(B, C) ===>       A
  // then |children_to_parents| will contain (A, [B, C])
  std::multimap<XY, XY> children_to_parents_ ABSL_GUARDED_BY(mu_);
  // and |parents_to_children|  will contain (B, A), (C, A)
  std::multimap<XY, XY> parents_to_children_ ABSL_GUARDED_BY(mu_);

  std::vector<UpdatedCb> updated_callbacks_{};

  // Metadata
  absl::optional<std::string> title_{std::nullopt};
  absl::optional<std::string> author_{std::nullopt};
  const absl::Time created_time_{absl::Now()};
  absl::Time edited_time_{absl::Now()};
};

} // namespace latis

#endif // SRC_LATIS_IMPL_H_
