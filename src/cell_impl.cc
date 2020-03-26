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

#include "src/cell_impl.h"

#include "google/protobuf/stubs/status.h"
#include "google/protobuf/stubs/statusor.h"

namespace latis {

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;

// StatusOr<CellObj> CellObj::FromAmount(XY xy, std::string_view input) {
//   // auto amount = fake_formula::ToAmount(input);
//   // if (!amount.ok()) {
//   //   return amount.status();
//   // }
//   // return CellObj(xy, amount.ValueOrDie());
// }
//
// StatusOr<CellObj> CellObj::FromFormula(XY xy, std::string input) {
//   auto expression = fake_formula::ToExpression(input);
//   if (!expression.ok()) {
//     return expression.status();
//   }
//   return CellObj(xy, expression.ValueOrDie());
// }
//
// StatusOr<CellObj> CellObj::From(XY xy, std::string input) {
//   // Try expression, then amount
//   if (const auto expression = FromFormula(xy, input); expression.ok()) {
//     return expression.ValueOrDie();
//   }
//   return FromAmount(xy, input);
// }

} // namespace latis
