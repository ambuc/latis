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

#include "src/test_utils/test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace {

TEST(IntegerToColumnLetter, Zero) {
  EXPECT_EQ(XY::IntegerToColumnLetter(0), "A");
}
TEST(IntegerToColumnLetter, One) {
  EXPECT_EQ(XY::IntegerToColumnLetter(1), "B");
}
TEST(IntegerToColumnLetter, TwentyFive) {
  EXPECT_EQ(XY::IntegerToColumnLetter(25), "Z");
}
TEST(IntegerToColumnLetter, TwentySix) {
  EXPECT_EQ(XY::IntegerToColumnLetter(26), "AA");
}
TEST(IntegerToColumnLetter, FiftyOne) {
  EXPECT_EQ(XY::IntegerToColumnLetter(51), "AZ");
}
TEST(IntegerToColumnLetter, FiftyTwo) {
  EXPECT_EQ(XY::IntegerToColumnLetter(52), "BA");
}
TEST(IntegerToColumnLetter, SevenOhOne) {
  EXPECT_EQ(XY::IntegerToColumnLetter(701), "ZZ");
}
TEST(IntegerToColumnLetter, SevenOhTwo) {
  EXPECT_EQ(XY::IntegerToColumnLetter(702), "AAA");
}

TEST(ColumnLetterToInteger, A) {
  EXPECT_THAT(XY::ColumnLetterToInteger("A"), IsOkAndHolds(0));
}
TEST(ColumnLetterToInteger, B) {
  EXPECT_THAT(XY::ColumnLetterToInteger("B"), IsOkAndHolds(1));
}
TEST(ColumnLetterToInteger, Z) {
  EXPECT_THAT(XY::ColumnLetterToInteger("Z"), IsOkAndHolds(25));
}
TEST(ColumnLetterToInteger, AA) {
  EXPECT_THAT(XY::ColumnLetterToInteger("AA"), IsOkAndHolds(26));
}
TEST(ColumnLetterToInteger, AB) {
  EXPECT_THAT(XY::ColumnLetterToInteger("AB"), IsOkAndHolds(27));
}
TEST(ColumnLetterToInteger, AZ) {
  EXPECT_THAT(XY::ColumnLetterToInteger("AZ"), IsOkAndHolds(51));
}
TEST(ColumnLetterToInteger, BA) {
  EXPECT_THAT(XY::ColumnLetterToInteger("BA"), IsOkAndHolds(52));
}
TEST(ColumnLetterToInteger, ZZ) {
  EXPECT_THAT(XY::ColumnLetterToInteger("ZZ"), IsOkAndHolds(701));
}
TEST(ColumnLetterToInteger, AAA) {
  EXPECT_THAT(XY::ColumnLetterToInteger("AAA"), IsOkAndHolds(702));
}

TEST(ColumnLetterToInteger, BunkEmpty) {
  EXPECT_FALSE(XY::ColumnLetterToInteger("").ok());
}
TEST(ColumnLetterToInteger, BunkUnderscore) {
  EXPECT_FALSE(XY::ColumnLetterToInteger("_").ok());
}
TEST(ColumnLetterToInteger, BunkLowerCaseA) {
  EXPECT_FALSE(XY::ColumnLetterToInteger("a").ok());
}
TEST(ColumnLetterToInteger, BunkPlus) {
  EXPECT_FALSE(XY::ColumnLetterToInteger("+").ok());
}
TEST(ColumnLetterToInteger, BunkTrailingSpace) {
  EXPECT_FALSE(XY::ColumnLetterToInteger("AAA ").ok());
}
TEST(ColumnLetterToInteger, BunkLeadingSpace) {
  EXPECT_FALSE(XY::ColumnLetterToInteger(" AAA").ok());
}
TEST(ColumnLetterToInteger, BunkInfixLowercase) {
  EXPECT_FALSE(XY::ColumnLetterToInteger("AaA").ok());
}
TEST(ColumnLetterToInteger, BunkInfixUnderscore) {
  EXPECT_FALSE(XY::ColumnLetterToInteger("A_A").ok());
}

TEST(XyToA1, ZeroByZero) { EXPECT_EQ(XY({0, 0}).ToA1(), "A1"); }
TEST(XyToA1, OneByZero) { EXPECT_EQ(XY({1, 0}).ToA1(), "B1"); }
TEST(XyToA1, ZeroByOne) { EXPECT_EQ(XY({0, 1}).ToA1(), "A2"); }
TEST(XyToA1, OneByOne) { EXPECT_EQ(XY({1, 1}).ToA1(), "B2"); }

TEST(A1ToXy, A1) { EXPECT_THAT(XY::From("A1"), IsOkAndHolds(XY(0, 0))); }
TEST(A1ToXy, A2) { EXPECT_THAT(XY::From("A2"), IsOkAndHolds(XY(0, 1))); }
TEST(A1ToXy, B1) { EXPECT_THAT(XY::From("B1"), IsOkAndHolds(XY(1, 0))); }
TEST(A1ToXy, B2) { EXPECT_THAT(XY::From("B2"), IsOkAndHolds(XY(1, 1))); }

TEST(A1ToXy, Empty) { EXPECT_FALSE(XY::From("").ok()); }
TEST(A1ToXy, A) { EXPECT_FALSE(XY::From("A").ok()); }
TEST(A1ToXy, B) { EXPECT_FALSE(XY::From("B").ok()); }
TEST(A1ToXy, One) { EXPECT_FALSE(XY::From("1").ok()); }
TEST(A1ToXy, Two) { EXPECT_FALSE(XY::From("2").ok()); }
TEST(A1ToXy, 1A) { EXPECT_FALSE(XY::From("1A").ok()); }

} // namespace
} // namespace latis
