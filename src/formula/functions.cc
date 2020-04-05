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
#include <functional>

namespace latis {
namespace formula {

using ::google::protobuf::Timestamp;
using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;
using ::google::protobuf::util::error::OK;

namespace {

// Numeric conversions.

bool IsNumeric(const Amount &a) {
  return a.has_int_amount() || a.has_double_amount();
}
double AsDouble(const Amount &a) {
  return a.double_amount() + static_cast<double>(a.int_amount());
}

// Money conversions.

double AsDouble(const Money &m) {
  return m.dollars() + (static_cast<double>(m.cents()) / 100.0);
}

Money AsMoney(double d) {
  Money resultant;
  if (d < 0) {
    resultant.set_dollars(std::ceil(d));
    resultant.set_cents(
        static_cast<int>(std::round((d - std::ceil(d)) * 100.0)));
  } else {
    resultant.set_dollars(std::floor(d));
    resultant.set_cents(
        static_cast<int>(std::round((d - std::floor(d)) * 100.0)));
  }
  return resultant;
}

Status CheckSameCurrency(const Money &lhs, const Money &rhs) {
  if (lhs.currency() != rhs.currency()) {
    return Status(INVALID_ARGUMENT, "different currencies.");
  }
  return Status(OK, "");
}

// Amount conversions
Amount FromInt(int i) {
  Amount resultant;
  resultant.set_int_amount(i);
  return resultant;
}
Amount FromDouble(double d) {
  Amount resultant;
  resultant.set_double_amount(d);
  return resultant;
}

} // namespace

// TIMESTAMP

StatusOr<bool> operator<=(const Timestamp &lhs, const Timestamp &rhs) {
  return lhs.seconds() <= rhs.seconds() && lhs.nanos() <= rhs.nanos();
}

StatusOr<Timestamp> operator+(const Timestamp &lhs, const Timestamp &rhs) {
  Timestamp resultant;
  resultant.set_seconds(lhs.seconds() + rhs.seconds());
  resultant.set_nanos(lhs.nanos() + rhs.nanos());
  return resultant;
}
Timestamp operator-(const Timestamp &arg) {
  Timestamp resultant;
  resultant.set_seconds(-arg.seconds());
  resultant.set_nanos(-arg.nanos());
  return resultant;
}

// MONEY

StatusOr<bool> operator<=(const Money &lhs, const Money &rhs) {
  return lhs.dollars() <= rhs.dollars() && lhs.cents() <= rhs.cents();
}

StatusOr<Money> operator+(const Money &lhs, const Money &rhs) {
  RETURN_IF_ERROR_(CheckSameCurrency(lhs, rhs));
  Money resultant = AsMoney(AsDouble(lhs) + AsDouble(rhs));
  resultant.set_currency(lhs.currency());
  return resultant;
}

Money operator-(const Money &arg) {
  Money resultant = AsMoney(-1.0 * AsDouble(arg));
  resultant.set_currency(arg.currency());
  return resultant;
}

StatusOr<Money> operator*(const Money &lhs, const Money &rhs) {
  RETURN_IF_ERROR_(CheckSameCurrency(lhs, rhs));
  Money resultant = AsMoney(AsDouble(lhs) * AsDouble(rhs));
  resultant.set_currency(lhs.currency());
  return resultant;
}

StatusOr<Money> operator/(const Money &lhs, const Money &rhs) {
  RETURN_IF_ERROR_(CheckSameCurrency(lhs, rhs));
  Money resultant = AsMoney(AsDouble(lhs) / AsDouble(rhs));
  resultant.set_currency(lhs.currency());
  return resultant;
}

// AMOUNTS

StatusOr<bool> operator<=(const Amount &lhs, const Amount &rhs) {
  if (lhs.has_str_amount() && rhs.has_str_amount()) {
    return lhs.str_amount() <= rhs.str_amount();
  } else if (lhs.has_timestamp_amount() && rhs.has_timestamp_amount()) {
    return lhs.timestamp_amount() <= rhs.timestamp_amount();
  } else if (lhs.has_money_amount() && rhs.has_money_amount()) {
    return lhs.money_amount() <= rhs.money_amount();
  } else if (lhs.has_int_amount() && rhs.has_int_amount()) {
    return lhs.int_amount() <= rhs.int_amount();
  } else if (lhs.has_bool_amount() && rhs.has_bool_amount()) {
    return lhs.bool_amount() <= rhs.bool_amount();
  } else if (IsNumeric(lhs) && IsNumeric(rhs)) {
    return AsDouble(lhs) <= AsDouble(rhs);
  }
  return Status(INVALID_ARGUMENT, "No operator<=() implemented.");
}

StatusOr<bool> operator==(const Amount &lhs, const Amount &rhs) {
  if (lhs.has_str_amount() && rhs.has_str_amount()) {
    return lhs.str_amount() == rhs.str_amount();
  } else if (lhs.has_timestamp_amount() && rhs.has_timestamp_amount()) {
    return lhs.timestamp_amount() == rhs.timestamp_amount();
  } else if (lhs.has_money_amount() && rhs.has_money_amount()) {
    return lhs.money_amount() == rhs.money_amount();
  } else if (lhs.has_int_amount() && rhs.has_int_amount()) {
    return lhs.int_amount() == rhs.int_amount();
  } else if (lhs.has_bool_amount() && rhs.has_bool_amount()) {
    return lhs.bool_amount() == rhs.bool_amount();
  } else if (IsNumeric(lhs) && IsNumeric(rhs)) {
    return AsDouble(lhs) == AsDouble(rhs);
  }
  return Status(INVALID_ARGUMENT, "No operator<=() implemented.");
}

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
  } else if (lhs.has_int_amount() && rhs.has_int_amount()) {
    return FromInt(lhs.int_amount() + rhs.int_amount());
  } else if (IsNumeric(lhs) && IsNumeric(rhs)) {
    // guaranteed not to double-count ;)
    return FromDouble(AsDouble(lhs) + AsDouble(rhs));
  }
  return Status(INVALID_ARGUMENT, "no sum");
}

StatusOr<Amount> operator-(const Amount &arg) {
  Amount resultant = arg;
  if (resultant.has_int_amount()) {
    return FromInt(-arg.int_amount());
  } else if (resultant.has_double_amount()) {
    return FromDouble(-arg.double_amount());
  } else if (resultant.has_money_amount()) {
    *resultant.mutable_money_amount() = -resultant.money_amount();
  } else if (resultant.has_timestamp_amount()) {
    *resultant.mutable_timestamp_amount() = -resultant.timestamp_amount();
  } else if (resultant.has_bool_amount()) {
    resultant.set_bool_amount(-resultant.bool_amount());
  } else if (resultant.has_str_amount()) {
    return Status(INVALID_ARGUMENT, "Can't negate a string.");
  }
  return resultant;
}

StatusOr<Amount> operator*(const Amount &lhs, const Amount &rhs) {
  Amount resultant;

  if (lhs.has_money_amount() && rhs.has_money_amount()) {
    ASSIGN_OR_RETURN_(*resultant.mutable_money_amount(),
                      lhs.money_amount() * rhs.money_amount());
    return resultant;
  } else if (lhs.has_int_amount() && rhs.has_int_amount()) {
    return FromInt(lhs.int_amount() * rhs.int_amount());
  } else if (IsNumeric(lhs) && IsNumeric(rhs)) {
    return FromDouble(AsDouble(lhs) * AsDouble(rhs));
  }

  return Status(INVALID_ARGUMENT, "no product");
}

StatusOr<Amount> operator/(const Amount &lhs, const Amount &rhs) {
  Amount resultant;

  if (lhs.has_money_amount() && rhs.has_money_amount()) {
    ASSIGN_OR_RETURN_(*resultant.mutable_money_amount(),
                      lhs.money_amount() / rhs.money_amount());
    return resultant;
  } else if (IsNumeric(lhs) && IsNumeric(rhs)) {
    return FromDouble(AsDouble(lhs) / AsDouble(rhs));
  }

  return Status(INVALID_ARGUMENT, "no division");
}

StatusOr<Amount> operator^(const Amount &lhs, const Amount &rhs) {
  Amount resultant;
  if (IsNumeric(lhs) && IsNumeric(rhs)) {
    return FromDouble(pow(AsDouble(lhs), AsDouble(rhs)));
  }
  return Status(INVALID_ARGUMENT, "no exponent");
}

StatusOr<Amount> operator%(const Amount &lhs, const Amount &rhs) {
  Amount resultant;
  if (lhs.has_int_amount() && rhs.has_int_amount()) {
    return FromInt(lhs.int_amount() % rhs.int_amount());
  } else if (IsNumeric(lhs) && IsNumeric(rhs)) {
    return FromDouble(fmod(AsDouble(lhs), AsDouble(rhs)));
  }
  return Status(INVALID_ARGUMENT, "no exponent");
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
