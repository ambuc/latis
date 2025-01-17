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

#ifndef SRC_FORMULA_FORMULA_H_
#define SRC_FORMULA_FORMULA_H_

#include "proto/latis_msg.pb.h"
#include "src/formula/common.h"

#include "absl/strings/str_format.h"
#include "google/protobuf/stubs/statusor.h"

namespace latis {
namespace formula {

// One-stop shop for lex, parse, and evaluate.
::google::protobuf::util::StatusOr<std::tuple<Expression, Amount>>
Parse(std::string_view input, const LookupFn &lookup_fn);

} // namespace formula
} // namespace latis

#endif // SRC_FORMULA_FORMULA_H_
