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
#include "src/metadata_interface.h"
#include "src/xy.h"

#include "absl/base/thread_annotations.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"

namespace latis {

class Latis : public LatisInterface {
public:
  Latis();
  Latis(const LatisMsg &sheet);

  // Getters
  CellObj *Get(XY xy) override;
  std::string Print(XY xy) const override;

  // Setters
  void Set(CellObj cell) override { cells_[cell.Xy()] = cell; }

  // Export
  LatisMsg Write() const override;

  // Kept structs
  MetadataInterface *Metadata() override { return metadata_.get(); }

private:
  void UpdateEditTime() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    absl::MutexLock l(&mu_);
    metadata_->UpdateEditTime();
  }

  mutable absl::Mutex mu_;

  std::unique_ptr<MetadataInterface> metadata_ ABSL_GUARDED_BY(mu_);
  absl::flat_hash_map<XY, CellObj> cells_ ABSL_GUARDED_BY(mu_);
  std::multimap<XY, XY> edges_ ABSL_GUARDED_BY(mu_);
};

} // namespace latis

#endif // SRC_LATIS_IMPL_H_
