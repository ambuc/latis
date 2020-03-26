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

#ifndef SRC_METADATA_IMPL_H_
#define SRC_METADATA_IMPL_H_

#include "src/metadata_interface.h"

#include "proto/latis_msg.pb.h"

#include "absl/time/clock.h"
#include "absl/time/time.h"

namespace latis {

// Class for keeping metadata. Not thread-safe, should be mutex-protected at
// usage site.
class MetadataImpl : public MetadataInterface {
public:
  MetadataImpl() {}
  MetadataImpl(const Metadata &metadata);
  std::optional<std::string> Title() const override { return title_; }
  std::optional<std::string> Author() const override { return author_; }
  absl::Time CreatedTime() const override { return created_time_; }
  absl::Time EditedTime() const override { return edited_time_; }
  void SetTitle(std::string title) override {
    UpdateEditTime();
    title_ = title;
  }
  void SetAuthor(std::string author) override {
    UpdateEditTime();
    author_ = author;
  }

  void UpdateEditTime() override { edited_time_ = absl::Now(); }

  void WriteTo(LatisMsg *latis_msgb) const override;

private:
  std::optional<std::string> title_{std::nullopt};
  std::optional<std::string> author_{std::nullopt};
  const absl::Time created_time_{absl::Now()};
  absl::Time edited_time_{absl::Now()};
};

} // namespace latis

#endif // SRC_METADATA_IMPL_H_
