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

#ifndef SRC_PARSER_COMMON_H_
#define SRC_PARSER_COMMON_H_

#include "absl/types/span.h"
#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/statusor.h"

namespace latis {
namespace formula {

// Helpful usings which make writing code in a header feel more natural.
using Status = ::google::protobuf::util::Status;

template <typename T> //
using StatusOr = ::google::protobuf::util::StatusOr<T>;

// no default ctor
struct Token {
  enum T {
    // Single character matches.
    equals,     // =
    period,     // .
    comma,      // ,
    lparen,     // (
    rparen,     // )
    plus,       // +
    minus,      // -
    asterisk,   // *
    slash,      // /
    carat,      // ^
    dollar,     // $
    percent,    // %
    tick,       // '
    lthan,      // <
    gthan,      // >
    question,   // ?
    colon,      // :
    underscore, // _

    // unusual
    literal, // \?, for escapint a single character

    numeric, // one or more 0-9
    alpha,   // one or more of a-z, A-Z,

    quote, // "\"" ALPHA_NUM "\""
  };
  T type;
  std::string_view value;
};

bool operator==(const Token &lhs, const Token &rhs) {
  return (lhs.type == rhs.type) && (lhs.value == rhs.value);
}

using TSpan = absl::Span<const Token>;

std::string PrintTSpan(TSpan *tspan) {
  std::string resultant;
  for (const auto token : *tspan) {
    resultant.append(token.value);
  }
  return resultant;
}
void PrintLnTSpan(TSpan *tspan) { std::cout << PrintTSpan(tspan) << std::endl; }

template <typename T> //
using Prsr = std::function<StatusOr<T>(TSpan *)>;

} // namespace formula
} // namespace latis

#endif // SRC_PARSER_COMMON_H_
