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

#include "src/display_utils.h"

#include "absl/functional/bind_front.h"
#include "absl/strings/str_format.h"
#include "absl/time/time.h"

namespace latis {

std::string PrintAmount(const Amount &amount, const FmtOptions &afo) {
  switch (amount.amount_demux_case()) {
  case Amount::AMOUNT_DEMUX_NOT_SET: {
    return "?";
  }
  case Amount::kStrAmount: {
    return absl::StrFormat("'%s'", amount.str_amount());
  }
  case Amount::kBoolAmount: {
    return amount.bool_amount() ? "True" : "False";
  }
  case Amount::kIntAmount: {
    return absl::StrFormat("%d", amount.int_amount());
  }
  case Amount::kDoubleAmount: {
    return absl::StrFormat("%.*f", afo.double_precision,
                           amount.double_amount());
  }
  case Amount::kTimestampAmount: {
    return absl::StrFormat("%s", absl::FormatTime(absl::FromUnixSeconds(
                                     amount.timestamp_amount().seconds())));
  }
  case Amount::kMoneyAmount: {
    switch (amount.money_amount().currency()) {
    case Money::USD: {
      return absl::StrFormat("$%d.%02d", amount.money_amount().dollars(),
                             amount.money_amount().cents());
    }
    default: {
      return "?";
    }
    }
  }
  }
  return "?";
}

std::string PrintAmount(const Amount &amount) {
  return PrintAmount(amount, FmtOptions());
}
std::string PrintCell(const Cell &cell, const FmtOptions &afo) {
  return PrintAmount(cell.formula().cached_amount(), afo);
}
std::string PrintCell(const Cell &cell) {
  return PrintCell(cell, FmtOptions());
}

} // namespace latis
