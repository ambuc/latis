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

#include <deque>

#include "src/status_utils/status_macros.h"

#include "absl/memory/memory.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"

namespace latis {
namespace formula {

using ::google::protobuf::util::Status;
using ::google::protobuf::util::StatusOr;
using ::google::protobuf::util::error::INVALID_ARGUMENT;
using ::google::protobuf::util::error::OK;

namespace {

// Returns a status: "Can't parse FOO as BAR."
Status CantParseAs(std::string_view *input, std::string_view as) {
  return Status(
      INVALID_ARGUMENT,
      absl::StrFormat("Couldn't parse %s as a token of type %s.", input, as));
}

// Returns a token with one-or-more characters off |input| matching |lambda| and
// fits them into a token of type |token_type|. Otherwise errors with |msg|.
StatusOr<Token> ExtractOneOrMore(Token::T token_type,
                                 std::function<bool(char)> lambda,
                                 std::string_view msg,
                                 std::string_view *input) {
  int n = 0;
  for (const char c : *input) {
    if (!lambda(c)) {
      break;
    }
    n++;
  }

  if (n == 0) {
    return CantParseAs(input, msg);
  }

  const auto value = input->substr(0, n);
  input->remove_prefix(n);

  return Token{.type = token_type, .value = value};
}

// Returns a token containing the first numeric character off |input|.
StatusOr<Token> AsNumeric(std::string_view *input) {
  return ExtractOneOrMore(
      Token::T::numeric,
      /*lambda=*/[](char c) -> bool { return isdigit(c); },
      /*msg=*/"NUMERIC", input);
}

// Returns a token containing the first alpha character off |input|.
StatusOr<Token> AsAlpha(std::string_view *input) {
  return ExtractOneOrMore(
      Token::T::alpha,
      /*lambda=*/[](char c) -> bool { return isalpha(c); },
      /*msg=*/"ALPHA", input);
}

// Returns a token containing a quoted string.
StatusOr<Token> AsQuote(std::string_view *input) {
  if (input->front() != '\"') {
    return CantParseAs(input, "Quote, first char isn't \".");
  }
  const auto len = input->find('\"', /*pos=*/1);
  if (len == std::string_view::npos) {
    return CantParseAs(input, "Quote, couldn't find second \".");
  }
  std::string_view value = input->substr(1, len - 1);
  input->remove_prefix(len + 1);
  return Token{.type = Token::T::quote, .value = value};
}

// Pops the head of |input| off as a token.
StatusOr<Token> AsToken(std::string_view *input) {
  if (input->empty()) {
    return Status(INVALID_ARGUMENT, "Is empty!");
  }

  const std::string_view cand = input->substr(0, 1);
  const char front = input->front();

  for (const auto [c, token] : std::vector<std::tuple<char, Token::T>>{
           {'=', Token::T::equals},
           {'.', Token::T::period},
           {',', Token::T::comma},
           {'(', Token::T::lparen},
           {')', Token::T::rparen},
           {'+', Token::T::plus},
           {'-', Token::T::minus},
           {'*', Token::T::asterisk},
           {'/', Token::T::slash},
           {'^', Token::T::carat},
           {'$', Token::T::dollar},
           {'%', Token::T::percent},
           {'\'', Token::T::tick},
           {'<', Token::T::lthan},
           {'>', Token::T::gthan},
           {'?', Token::T::question},
           {':', Token::T::colon},
           {'_', Token::T::underscore},
       }) {
    if (front == c) {
      input->remove_prefix(1);
      return Token{.type = token, .value = cand};
    }
  }

  if (front == '\\') {
    const std::string_view value = input->substr(1, 2);
    input->remove_prefix(2);
    return Token{.type = Token::T::literal, .value = value};
  }

  // Finally try matchers which require >1 char to work
  for (const auto lambda : {
           AsNumeric,
           AsAlpha,
           AsQuote,
       }) {
    if (const auto status_or_token = lambda(input); status_or_token.ok()) {
      return status_or_token.ValueOrDie();
    }
  }

  return Status(INVALID_ARGUMENT, "?");
}

} // namespace

StatusOr<std::vector<Token>> Lex(std::string_view s) {
  std::string_view mutable_s(s);
  std::vector<Token> tokens;

  while (true) {
    // Break if empty.
    if (mutable_s.empty()) {
      break;
    }

    // Ignore whitespace.
    if (mutable_s.front() == ' ') {
      mutable_s.remove_prefix(1);
      continue;
    }

    Token t;
    ASSIGN_OR_RETURN_(t, AsToken(&mutable_s));
    tokens.push_back(t);
  }

  return tokens;
}

} // namespace formula
} //  namespace latis
