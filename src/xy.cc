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

#include "src/xy.h"

#include "absl/strings/str_cat.h"
#include "google/protobuf/stubs/status_macros.h"
#include <math.h>

namespace latis {

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;

const int kCapitalLetterA = (int)'A';
const int kCapitalLetterZ = (int)'Z';

XY XY::From(std::tuple<int, int> tuple) {
  return XY(std::get<0>(tuple), std::get<1>(tuple));
}

XY XY::From(PointLocation pl) { return XY(pl.col(), pl.row()); }

StatusOr<XY> XY::From(std::string_view a1) {
  if (a1.empty()) {
    return Status(INVALID_ARGUMENT, "XY::From() given empty!");
  }

  int idx = 0; // index of 1st [0-9]
  while (isupper(a1[idx])) {
    idx++;
  }

  if (idx == 0 || idx > (int)a1.size()) {
    return Status(INVALID_ARGUMENT,
                  "argument to XY::From() must start with uppercase letters.");
  }

  const auto col_or_status = XY::ColumnLetterToInteger(a1.substr(0, idx));
  if (!col_or_status.ok()) {
    return col_or_status.status();
  }

  int row = 0;
  if (!absl::SimpleAtoi(a1.substr(idx, a1.size()), &row)) {
    return Status(INVALID_ARGUMENT,
                  "argument to XY::From() must end with valid number");
  }
  row--;

  return XY(col_or_status.ValueOrDie(), row);
}

PointLocation XY::ToPointLocation() const {
  PointLocation pl;
  pl.set_col(y_);
  pl.set_row(x_);
  return pl;
}

std::string XY::ToA1() const {
  return absl::StrCat(IntegerToColumnLetter(x_), std::to_string(y_ + 1));
}

std::string XY::ToColumnLetter() const { return XY::IntegerToColumnLetter(x_); }

StatusOr<int> XY::ColumnLetterToInteger(std::string_view s) {
  if (s.empty()) {
    return Status(INVALID_ARGUMENT, "cltoint given empty!");
  }
  if (s.back() < kCapitalLetterA || kCapitalLetterZ < s.back()) {
    return Status(INVALID_ARGUMENT, "Encountered a character not in [A-Z].");
  }

  int val = (int)(s.back()) - kCapitalLetterA;

  s.remove_suffix(1);

  if (s.empty()) {
    return val;
  }

  if (const auto rest = XY::ColumnLetterToInteger(s); rest.ok()) {
    return val + 26 * (1 + rest.ValueOrDie());
  } else {
    return rest.status();
  }
}

std::string XY::IntegerToColumnLetter(int i) {
  std::string v;
  if (const int r = i / 26; r > 0) {
    v.append(IntegerToColumnLetter(r - 1));
  }
  v.push_back((char)(kCapitalLetterA + (i % 26)));
  return v;
}

} // namespace latis
