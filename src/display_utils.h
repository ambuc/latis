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

#ifndef SRC_DISPLAY_UTILS_H_
#define SRC_DISPLAY_UTILS_H_

#include "proto/latis_msg.pb.h"
#include "src/xy.h"

#include <iostream>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"

namespace latis {

struct FmtOptions {
  int width = 5;
  int double_precision = 3;
};

std::string PrintCell(const Cell &cell, const FmtOptions &afo);
std::string PrintCell(const Cell &cell);
std::string PrintAmount(const Amount &amount, const FmtOptions &afo);
std::string PrintAmount(const Amount &amount);

} // namespace latis

#endif // SRC_DISPLAY_UTILS_H_
