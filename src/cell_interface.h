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

#ifndef SRC_CELL_INTERFACE_H_
#define SRC_CELL_INTERFACE_H_

#include "proto/latis_msg.pb.h"
#include "src/xy.h"

#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/status_macros.h"
#include "google/protobuf/stubs/statusor.h"

namespace latis {

class CellInterface {
public:
  // Getter
  virtual XY Xy() const = 0;
  virtual Amount GetAmount() const = 0;
  // Export
  virtual Cell Export() const = 0;
  // Display
  virtual std::string Print() const = 0;
};

} // namespace latis

#endif // SRC_CELL_INTERFACE_H_
