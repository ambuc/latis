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

#include "src/formula/functions.h"

#include "src/utils/status_macros.h"
#include <cmath>

namespace latis {
namespace formula {

using ::google::protobuf::Timestamp;
using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;
using ::google::protobuf::util::error::OK;

// TIMESTAMP

bool operator<(const Timestamp &lhs, const Timestamp &rhs) {
  return lhs.seconds() < rhs.seconds() && lhs.nanos() < rhs.nanos();
}
bool operator>(const Timestamp &lhs, const Timestamp &rhs) {
  return lhs.seconds() > rhs.seconds() && lhs.nanos() > rhs.nanos();
}
bool operator==(const Timestamp &lhs, const Timestamp &rhs) {
  return lhs.seconds() == rhs.seconds() && lhs.seconds() == rhs.seconds();
}
bool operator<=(const Timestamp &lhs, const Timestamp &rhs) {
  return lhs < rhs || lhs == rhs;
}
bool operator>=(const Timestamp &lhs, const Timestamp &rhs) {
  return lhs > rhs || lhs == rhs;
}
StatusOr<Timestamp> operator+(const Timestamp &lhs, const Timestamp &rhs) {
  Timestamp resultant;
  resultant.set_seconds(lhs.seconds() + rhs.seconds());
  resultant.set_nanos(lhs.nanos() + rhs.nanos());
  return resultant;
}
StatusOr<Timestamp> operator-(const Timestamp &lhs, const Timestamp &rhs) {
  if (lhs < rhs) {
    return Status(INVALID_ARGUMENT, "Timestamp lhs < rhs, can't subtract.");
  }
  Timestamp resultant;
  resultant.set_seconds(lhs.seconds() - rhs.seconds());
  resultant.set_nanos(lhs.nanos() - rhs.nanos());
  return resultant;
}

// MONEY

bool operator<(const Money &lhs, const Money &rhs) {
  return lhs.dollars() < rhs.dollars() && lhs.dollars() < rhs.dollars();
}
bool operator>(const Money &lhs, const Money &rhs) {
  return lhs.dollars() > rhs.dollars() && lhs.dollars() > rhs.dollars();
}
bool operator==(const Money &lhs, const Money &rhs) {
  return lhs.dollars() == rhs.dollars() && lhs.dollars() == rhs.dollars();
}
bool operator<=(const Money &lhs, const Money &rhs) {
  return lhs < rhs || lhs == rhs;
}
bool operator>=(const Money &lhs, const Money &rhs) {
  return lhs > rhs || lhs == rhs;
}
StatusOr<Money> operator+(const Money &lhs, const Money &rhs) {
  if (lhs.currency() != rhs.currency()) {
    return Status(INVALID_ARGUMENT, "different currencies.");
  }
  Money resultant;
  resultant.set_currency(lhs.currency());
  resultant.set_dollars(lhs.dollars() + rhs.dollars());
  resultant.set_cents(lhs.cents() + rhs.cents());
  return resultant;
}
StatusOr<Money> operator-(const Money &lhs, const Money &rhs) {
  if (lhs.currency() != rhs.currency()) {
    return Status(INVALID_ARGUMENT, "different currencies.");
  }
  if (lhs < rhs) {
    return Status(INVALID_ARGUMENT, "lhs < rhs, can't subtract money.");
  }
  Money resultant;
  resultant.set_currency(lhs.currency());
  resultant.set_dollars(lhs.dollars() - rhs.dollars());
  resultant.set_cents(lhs.cents() - rhs.cents());
  return resultant;
}
StatusOr<Money> operator*(const Money &lhs, const Money &rhs) {
  if (lhs.currency() != rhs.currency()) {
    return Status(INVALID_ARGUMENT, "different currencies.");
  }
  Money resultant;
  resultant.set_currency(lhs.currency());
  double lhs_d = lhs.dollars() + (static_cast<double>(lhs.cents()) / 100.0);
  double rhs_d = rhs.dollars() + (static_cast<double>(rhs.cents()) / 100.0);
  double resultant_d = lhs_d * rhs_d;
  int dollars = std::floor(resultant_d);
  resultant.set_dollars(dollars);
  resultant.set_cents(
      static_cast<int>(std::round((resultant_d - dollars) * 100.0)));
  return resultant;
}
StatusOr<Money> operator/(const Money &lhs, const Money &rhs) {
  if (lhs.currency() != rhs.currency()) {
    return Status(INVALID_ARGUMENT, "different currencies.");
  }
  Money resultant;
  resultant.set_currency(lhs.currency());
  double lhs_d = lhs.dollars() + (static_cast<double>(lhs.cents()) / 100.0);
  double rhs_d = rhs.dollars() + (static_cast<double>(rhs.cents()) / 100.0);
  double resultant_d = lhs_d / rhs_d;
  int dollars = std::floor(resultant_d);
  resultant.set_dollars(dollars);
  resultant.set_cents(
      static_cast<int>(std::round((resultant_d - dollars) * 100.0)));
  return resultant;
}

// AMOUNTS

StatusOr<Amount> operator+(const Amount &lhs, const Amount &rhs) {
  Amount resultant;

  if (lhs.has_str_amount() && rhs.has_str_amount()) {
    resultant = lhs;
    resultant.mutable_str_amount()->append(rhs.str_amount());
    return resultant;
  } else if (lhs.has_timestamp_amount() && rhs.has_timestamp_amount()) {
    ASSIGN_OR_RETURN_(*resultant.mutable_timestamp_amount(),
                      lhs.timestamp_amount() + rhs.timestamp_amount());
    return resultant;
  } else if (lhs.has_money_amount() && rhs.has_money_amount()) {
    ASSIGN_OR_RETURN_(*resultant.mutable_money_amount(),
                      lhs.money_amount() + rhs.money_amount());
    return resultant;
  } else if (lhs.has_int_amount()) {
    if (rhs.has_int_amount()) {
      resultant.set_int_amount(lhs.int_amount() + rhs.int_amount());
      return resultant;
    } else if (rhs.has_double_amount()) {
      resultant.set_double_amount(static_cast<double>(lhs.int_amount()) +
                                  rhs.double_amount());
      return resultant;
    }
  } else if (lhs.has_double_amount()) {
    if (rhs.has_double_amount()) {
      resultant.set_double_amount(lhs.double_amount() + rhs.double_amount());
      return resultant;
    } else if (rhs.has_int_amount()) {
      resultant.set_double_amount(lhs.double_amount() +
                                  static_cast<double>(rhs.int_amount()));
      return resultant;
    }
  }
  return Status(INVALID_ARGUMENT, "no sum");
}

StatusOr<Amount> operator-(const Amount &lhs, const Amount &rhs) {
  Amount resultant;

  if (lhs.has_timestamp_amount() && rhs.has_timestamp_amount()) {
    ASSIGN_OR_RETURN_(*resultant.mutable_timestamp_amount(),
                      lhs.timestamp_amount() - rhs.timestamp_amount());
    return resultant;
  } else if (lhs.has_money_amount() && rhs.has_money_amount()) {
    ASSIGN_OR_RETURN_(*resultant.mutable_money_amount(),
                      lhs.money_amount() - rhs.money_amount());
    return resultant;
  } else if (lhs.has_int_amount()) {
    if (rhs.has_int_amount()) {
      resultant.set_int_amount(lhs.int_amount() - rhs.int_amount());
      return resultant;
    } else if (rhs.has_double_amount()) {
      resultant.set_double_amount(static_cast<double>(lhs.int_amount()) -
                                  rhs.double_amount());
      return resultant;
    }
  } else if (lhs.has_double_amount()) {
    if (rhs.has_double_amount()) {
      resultant.set_double_amount(lhs.double_amount() - rhs.double_amount());
      return resultant;
    } else if (rhs.has_int_amount()) {
      resultant.set_double_amount(lhs.double_amount() -
                                  static_cast<double>(rhs.int_amount()));
      return resultant;
    }
  }

  return Status(INVALID_ARGUMENT, "no difference");
}
StatusOr<Amount> operator*(const Amount &lhs, const Amount &rhs) {
  Amount resultant;

  if (lhs.has_money_amount() && rhs.has_money_amount()) {
    ASSIGN_OR_RETURN_(*resultant.mutable_money_amount(),
                      lhs.money_amount() * rhs.money_amount());
    return resultant;
  } else if (lhs.has_int_amount()) {
    if (rhs.has_int_amount()) {
      resultant.set_int_amount(lhs.int_amount() * rhs.int_amount());
      return resultant;
    } else if (rhs.has_double_amount()) {
      resultant.set_double_amount(static_cast<double>(lhs.int_amount()) *
                                  rhs.double_amount());
      return resultant;
    }
  } else if (lhs.has_double_amount()) {
    if (rhs.has_double_amount()) {
      resultant.set_double_amount(lhs.double_amount() * rhs.double_amount());
      return resultant;
    } else if (rhs.has_int_amount()) {
      resultant.set_double_amount(lhs.double_amount() *
                                  static_cast<double>(rhs.int_amount()));
      return resultant;
    }
  }

  return Status(INVALID_ARGUMENT, "no product");
}
StatusOr<Amount> operator/(const Amount &lhs, const Amount &rhs) {
  Amount resultant;

  if (lhs.has_money_amount() && rhs.has_money_amount()) {
    ASSIGN_OR_RETURN_(*resultant.mutable_money_amount(),
                      lhs.money_amount() / rhs.money_amount());
    return resultant;
  } else if (lhs.has_int_amount()) {
    if (rhs.has_int_amount()) {
      resultant.set_int_amount(lhs.int_amount() / rhs.int_amount());
      return resultant;
    } else if (rhs.has_double_amount()) {
      resultant.set_double_amount(static_cast<double>(lhs.int_amount()) /
                                  rhs.double_amount());
      return resultant;
    }
  } else if (lhs.has_double_amount()) {
    if (rhs.has_double_amount()) {
      resultant.set_double_amount(lhs.double_amount() / rhs.double_amount());
      return resultant;
    } else if (rhs.has_int_amount()) {
      resultant.set_double_amount(lhs.double_amount() /
                                  static_cast<double>(rhs.int_amount()));
      return resultant;
    }
  }

  return Status(INVALID_ARGUMENT, "no division");
}

StatusOr<Amount> operator&&(const Amount &lhs, const Amount &rhs) {
  Amount resultant;
  if (lhs.has_bool_amount() && rhs.has_bool_amount()) {
    resultant.set_bool_amount(lhs.bool_amount() && rhs.bool_amount());
    return resultant;
  }
  return Status(INVALID_ARGUMENT, "Can't && non-bools.");
}

StatusOr<Amount> operator||(const Amount &lhs, const Amount &rhs) {
  Amount resultant;
  if (lhs.has_bool_amount() && rhs.has_bool_amount()) {
    resultant.set_bool_amount(lhs.bool_amount() || rhs.bool_amount());
    return resultant;
  }
  return Status(INVALID_ARGUMENT, "Can't || non-bools.");
}

StatusOr<Amount> operator!(const Amount &arg) {
  Amount resultant;
  if (arg.has_bool_amount()) {
    resultant.set_bool_amount(!arg.bool_amount());
    return resultant;
  }
  return Status(INVALID_ARGUMENT, "Can't ! non-bools.");
}

} // namespace formula
} // namespace latis
