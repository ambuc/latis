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

#ifndef SRC_FORMULA_LEXER_H_
#define SRC_FORMULA_LEXER_H_

#include "proto/latis_msg.pb.h"

#include "src/formula/common.h"

#include "absl/types/optional.h"
#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/status_macros.h"
#include "google/protobuf/stubs/statusor.h"

namespace latis {
namespace formula {

// Given an input string |s|, populates |tokens| with a list of tokens.
// Returns a non-OkStatus if there was an error.
::google::protobuf::util::StatusOr<std::vector<Token>> Lex(std::string_view s);

} // namespace formula
} // namespace latis

#endif // SRC_FORMULA_LEXER_H_
