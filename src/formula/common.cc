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

#include "src/formula/common.h"

namespace latis {
namespace formula {

bool operator==(const Token &lhs, const Token &rhs) {
  return (lhs.type == rhs.type) && (lhs.value == rhs.value);
}

std::string PrintTSpan(TSpan *tspan) {
  std::string resultant;
  for (const auto token : *tspan) {
    resultant.append(token.value);
  }
  return resultant;
}

void PrintLnTSpan(TSpan *tspan) { std::cout << PrintTSpan(tspan) << std::endl; }

} // namespace formula
} // namespace latis
