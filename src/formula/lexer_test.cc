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

#include "src/formula/lexer.h"

#include "absl/memory/memory.h"

#include "src/test_utils/test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace formula {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Matches;
using ::testing::Not;

MATCHER_P(IsToken, t, "") { return arg.type == t; }
MATCHER(IsTokenCarat, "") { return Matches(IsToken(Token::T::carat))(arg); }
MATCHER(IsTokenComma, "") { return Matches(IsToken(Token::T::comma))(arg); }
MATCHER(IsTokenDollar, "") { return Matches(IsToken(Token::T::dollar))(arg); }
MATCHER(IsTokenEquals, "") { return Matches(IsToken(Token::T::equals))(arg); }
MATCHER(IsTokenLparen, "") { return Matches(IsToken(Token::T::lparen))(arg); }
MATCHER(IsTokenMinus, "") { return Matches(IsToken(Token::T::minus))(arg); }
MATCHER(IsTokenPeriod, "") { return Matches(IsToken(Token::T::period))(arg); }
MATCHER(IsTokenRparen, "") { return Matches(IsToken(Token::T::rparen))(arg); }
MATCHER(IsTokenSlash, "") { return Matches(IsToken(Token::T::slash))(arg); }
MATCHER(IsTokenTick, "") { return Matches(IsToken(Token::T::tick))(arg); }
MATCHER_P(IsTokenAlpha, s, "") {
  return Matches(IsToken(Token::T::alpha))(arg) && (arg.value == s);
}
MATCHER_P(IsTokenEscape, c, "") {
  return Matches(IsToken(Token::T::literal))(arg) && (arg.value == c);
}
MATCHER_P(IsTokenNumeric, i, "") {
  return Matches(IsToken(Token::T::numeric))(arg) &&
         (arg.value == std::to_string(i));
}
MATCHER_P(IsTokenQuote, str, "") {
  return Matches(IsToken(Token::T::quote))(arg) && (arg.value == str);
}

class LexerTestClass : public ::testing::Test {};

TEST_F(LexerTestClass, EqualsFour) {
  EXPECT_THAT(Lex("4"), IsOkAndHolds(ElementsAre(IsTokenNumeric(4))));
}

TEST_F(LexerTestClass, EqualsEscapedChar) {
  EXPECT_THAT(Lex("\\c"), IsOkAndHolds(ElementsAre(IsTokenEscape("c"))));
}

TEST_F(LexerTestClass, EqualsEscapedQuote) {
  EXPECT_THAT(Lex("\\\""), IsOkAndHolds(ElementsAre(IsTokenEscape("\""))));
}

TEST_F(LexerTestClass, EqualsFourPointSix) {
  EXPECT_THAT(Lex("=4.605"),
              IsOkAndHolds(ElementsAre(IsTokenEquals(), IsTokenNumeric(4),
                                       IsTokenPeriod(), IsTokenNumeric(605))));
}

TEST_F(LexerTestClass, WhitespaceIgnored) {
  auto tokens_1 = Lex("=POW(4.605,\"foo\")").ValueOrDie();
  auto tokens_2 = Lex(" = POW ( 4.605 , \"foo\" ) ").ValueOrDie();
  EXPECT_THAT(tokens_1, ::testing::ContainerEq(tokens_2));
}

TEST_F(LexerTestClass, EqualsSomething) {
  EXPECT_THAT(Lex("=FOO(bar,4.0)"),
              IsOkAndHolds(ElementsAre(
                  IsTokenEquals(), IsTokenAlpha("FOO"), IsTokenLparen(),
                  IsTokenAlpha("bar"), IsTokenComma(), IsTokenNumeric(4),
                  IsTokenPeriod(), IsTokenNumeric(0), IsTokenRparen())));
}

TEST_F(LexerTestClass, EqualsQuote) {
  EXPECT_THAT(Lex("\"FOO BAR\""),
              IsOkAndHolds(ElementsAre(IsTokenQuote("FOO BAR"))));
}

TEST_F(LexerTestClass, EqualsFnOfQuote) {
  EXPECT_THAT(Lex("=POW(\"FOO _ 123 * 456 ) BAR\")"),
              IsOkAndHolds(ElementsAre(
                  IsTokenEquals(), IsTokenAlpha("POW"), IsTokenLparen(),
                  IsTokenQuote("FOO _ 123 * 456 ) BAR"), IsTokenRparen())));
}

TEST_F(LexerTestClass, EqualsSomethingComplicated) {
  EXPECT_THAT(
      Lex("=Pow(10^2,A1-21.43/7,$5,'foo,bar')"),
      IsOkAndHolds(ElementsAre(
          IsTokenEquals(), IsTokenAlpha("Pow"), IsTokenLparen(),
          IsTokenNumeric(10), IsTokenCarat(), IsTokenNumeric(2), IsTokenComma(),
          IsTokenAlpha("A"), IsTokenNumeric(1), IsTokenMinus(),
          IsTokenNumeric(21), IsTokenPeriod(), IsTokenNumeric(43),
          IsTokenSlash(), IsTokenNumeric(7), IsTokenComma(), IsTokenDollar(),
          IsTokenNumeric(5), IsTokenComma(), IsTokenTick(), IsTokenAlpha("foo"),
          IsTokenComma(), IsTokenAlpha("bar"), IsTokenTick(),
          IsTokenRparen())));
}

} // namespace
} // namespace formula
} // namespace latis
