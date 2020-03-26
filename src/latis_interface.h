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

#ifndef SRC_LATIS_INTERFACE_H_
#define SRC_LATIS_INTERFACE_H_

#include "proto/latis_msg.pb.h"
#include "src/cell_impl.h"
#include "src/metadata_interface.h"
#include "src/xy.h"

namespace latis {

class LatisInterface {
public:
  // Getters
  virtual CellObj *Get(XY xy) = 0;
  virtual std::string Print(XY xy) const = 0;

  // Setters
  virtual void Set(CellObj cell) = 0;

  // Export
  virtual LatisMsg Write() const = 0;

  // Kept structs
  virtual MetadataInterface *Metadata() = 0;
};

} // namespace latis

#endif // SRC_LATIS_INTERFACE_H_
