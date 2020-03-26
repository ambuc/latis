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

#include "src/metadata_impl.h"

#include "absl/time/time.h"

namespace latis {

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;

MetadataImpl::MetadataImpl(const Metadata &metadata)
    : title_(metadata.has_title() ? std::optional<std::string>(metadata.title())
                                  : std::nullopt),
      author_(metadata.has_author()
                  ? std::optional<std::string>(metadata.author())
                  : std::nullopt),
      created_time_(
          metadata.has_created_time()
              ? absl::FromUnixSeconds(metadata.created_time().seconds())
              : absl::Now()),
      edited_time_(metadata.has_edited_time()
                       ? absl::FromUnixSeconds(metadata.edited_time().seconds())
                       : absl::Now()) {}

void MetadataImpl::WriteTo(LatisMsg *latis_msgb) const {
  if (title_.has_value()) {
    latis_msgb->mutable_metadata()->set_title(title_.value());
  }
  if (author_.has_value()) {
    latis_msgb->mutable_metadata()->set_author(author_.value());
  }
  latis_msgb->mutable_metadata()->mutable_created_time()->set_seconds(
      absl::ToUnixSeconds(created_time_));
  latis_msgb->mutable_metadata()->mutable_edited_time()->set_seconds(
      absl::ToUnixSeconds(edited_time_));
}

} // namespace latis
