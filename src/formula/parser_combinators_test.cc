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

#include "src/formula/parser_combinators.h"

#include "src/test_utils/test_utils.h"

#include "absl/memory/memory.h"
#include "absl/time/time.h"
#include "google/protobuf/text_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace formula {
namespace {

using ::google::protobuf::TextFormat;
using ::google::protobuf::util::error::INVALID_ARGUMENT;
using ::testing::ContainerEq;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::Matches;
using ::testing::MockFunction;
using ::testing::Not;
using ::testing::Return;
using ::testing::VariantWith;

TEST(AnyVariant, AB_SucceedsOnA) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  MockFunction<StatusOr<double>(TSpan *)> MockB;
  EXPECT_CALL(MockA, Call).Times(1).WillOnce(Return(2));
  EXPECT_CALL(MockB, Call).Times(0);

  TSpan tspan;
  EXPECT_THAT((AnyVariant<int, double>(MockA.AsStdFunction(),
                                       MockB.AsStdFunction())(&tspan)),
              IsOkAndHolds(VariantWith<int>(2)));
}

TEST(AnyVariant, AB_SucceedsOnB) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  MockFunction<StatusOr<double>(TSpan *)> MockB;
  EXPECT_CALL(MockA, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));
  EXPECT_CALL(MockB, Call).Times(1).WillOnce(Return(2.0));

  TSpan tspan;
  EXPECT_THAT((AnyVariant<int, double>(MockA.AsStdFunction(),
                                       MockB.AsStdFunction())(&tspan)),
              IsOkAndHolds(VariantWith<double>(2.0)));
}

TEST(AnyVariant, AB_Fails) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  MockFunction<StatusOr<double>(TSpan *)> MockB;
  EXPECT_CALL(MockA, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));
  EXPECT_CALL(MockB, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));

  TSpan tspan;
  EXPECT_THAT((AnyVariant<int, double>(MockA.AsStdFunction(),
                                       MockB.AsStdFunction())(&tspan)),
              Not(IsOk()));
}

TEST(Any, AB_SucceedsOnA) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  MockFunction<StatusOr<int>(TSpan *)> MockB;
  EXPECT_CALL(MockA, Call).Times(1).WillOnce(Return(1));
  EXPECT_CALL(MockB, Call).Times(0);

  TSpan tspan;
  EXPECT_THAT(
      (Any<int>({MockA.AsStdFunction(), MockB.AsStdFunction()})(&tspan)),
      IsOkAndHolds(1));
}

TEST(Any, AB_SucceedsOnB) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  MockFunction<StatusOr<int>(TSpan *)> MockB;
  EXPECT_CALL(MockA, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));
  EXPECT_CALL(MockB, Call).Times(1).WillOnce(Return(1));

  TSpan tspan;
  EXPECT_THAT(
      (Any<int>({MockA.AsStdFunction(), MockB.AsStdFunction()})(&tspan)),
      IsOkAndHolds(1));
}

TEST(Any, AB_Fails) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  MockFunction<StatusOr<int>(TSpan *)> MockB;
  EXPECT_CALL(MockA, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));
  EXPECT_CALL(MockB, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));

  TSpan tspan;
  EXPECT_THAT(
      (Any<int>({MockA.AsStdFunction(), MockB.AsStdFunction()})(&tspan)),
      Not(IsOk()));
}

TEST(Maybe, Succeeds) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  EXPECT_CALL(MockA, Call).Times(1).WillOnce(Return(1));

  TSpan tspan;
  EXPECT_EQ((Maybe<int>(MockA.AsStdFunction())(&tspan)), 1);
}

TEST(Maybe, Fails) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  EXPECT_CALL(MockA, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));

  TSpan tspan;
  EXPECT_EQ((Maybe<int>(MockA.AsStdFunction())(&tspan)), absl::nullopt);
}

TEST(WithRestriction, ValueAndPasses) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  EXPECT_CALL(MockA, Call).Times(1).WillOnce(Return(0));

  auto restriction = [](int i) { return true; };

  TSpan tspan;
  EXPECT_THAT(
      (WithRestriction<int>(restriction)(MockA.AsStdFunction())(&tspan)),
      IsOkAndHolds(0));
}

TEST(WithRestriction, ValueAndFails) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  EXPECT_CALL(MockA, Call).Times(1).WillOnce(Return(0));

  auto restriction = [](int i) { return false; };

  TSpan tspan;
  EXPECT_THAT(
      (WithRestriction<int>(restriction)(MockA.AsStdFunction())(&tspan)),
      Not(IsOk()));
}

TEST(WithRestriction, NoValue) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  EXPECT_CALL(MockA, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));

  MockFunction<bool(int)> MockR;
  EXPECT_CALL(MockR, Call).Times(0);

  TSpan tspan;
  EXPECT_THAT((WithRestriction<int>(MockR.AsStdFunction())(
                  MockA.AsStdFunction())(&tspan)),
              Not(IsOk()));
}

TEST(InSequence, AThenB) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  MockFunction<StatusOr<std::string>(TSpan *)> MockB;

  EXPECT_CALL(MockA, Call).Times(1).WillOnce(Return(2));
  const std::string value = "two";
  EXPECT_CALL(MockB, Call).Times(1).WillOnce(Return(value));

  TSpan tspan;

  StatusOr<std::tuple<int, std::string>> result_or_status =
      InSequence<int, std::string>(MockA.AsStdFunction(),
                                   MockB.AsStdFunction())(&tspan);

  ASSERT_THAT(result_or_status, IsOk());

  std::tuple<int, std::string> result = result_or_status.ValueOrDie();

  EXPECT_THAT(std::get<0>(result), Eq(2));
  EXPECT_THAT(std::get<1>(result), Eq(value));
}

TEST(InSequence, AThenBFails) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  MockFunction<StatusOr<std::string>(TSpan *)> MockB;

  EXPECT_CALL(MockA, Call).Times(1).WillOnce(Return(2));
  EXPECT_CALL(MockB, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));

  TSpan tspan;

  EXPECT_THAT((InSequence<int, std::string>(MockA.AsStdFunction(),
                                            MockB.AsStdFunction())(&tspan)),
              Not(IsOk()));
}

TEST(InSequence, AFails) {
  MockFunction<StatusOr<int>(TSpan *)> MockA;
  MockFunction<StatusOr<std::string>(TSpan *)> MockB;

  EXPECT_CALL(MockA, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));
  // Never called.
  EXPECT_CALL(MockB, Call).Times(0);

  TSpan tspan;

  EXPECT_THAT((InSequence<int, std::string>(MockA.AsStdFunction(),
                                            MockB.AsStdFunction())(&tspan)),
              Not(IsOk()));
}

TEST(WithTransformation, AToB_AExists) {
  MockFunction<StatusOr<int>(TSpan *)> MockParser;
  EXPECT_CALL(MockParser, Call).Times(1).WillOnce(Return(2));

  MockFunction<bool(int)> MockTransformation;
  EXPECT_CALL(MockTransformation, Call(2)).Times(1).WillOnce(Return(true));

  TSpan tspan;
  EXPECT_THAT(
      (WithTransformation<int, bool>(MockTransformation.AsStdFunction())(
          MockParser.AsStdFunction())(&tspan)),
      IsOkAndHolds(true));
}

TEST(WithTransformation, AToB_ADoesntExist) {
  MockFunction<StatusOr<int>(TSpan *)> MockParser;
  EXPECT_CALL(MockParser, Call)
      .Times(1)
      .WillOnce(Return(Status(INVALID_ARGUMENT, "")));

  MockFunction<bool(int)> MockTransformation;
  EXPECT_CALL(MockTransformation, Call).Times(0);

  TSpan tspan;
  EXPECT_THAT(
      (WithTransformation<int, bool>(MockTransformation.AsStdFunction())(
          MockParser.AsStdFunction())(&tspan)),
      Not(IsOk()));
}

TEST(WithLookup, AToB_Found) {
  MockFunction<StatusOr<int>(TSpan *)> MockParser;
  EXPECT_CALL(MockParser, Call).Times(1).WillOnce(Return(42));
  std::unordered_map<int, double> map{{42, 123.456}};

  TSpan tspan;
  EXPECT_THAT((WithLookup(map)(MockParser.AsStdFunction())(&tspan)),
              IsOkAndHolds(Eq(123.456)));
}

TEST(WithLookup, AToB_NotFound) {
  MockFunction<StatusOr<int>(TSpan *)> MockParser;
  EXPECT_CALL(MockParser, Call).Times(1).WillOnce(Return(42));
  std::unordered_map<int, double> empty_map{};

  TSpan tspan;
  EXPECT_THAT((WithLookup(empty_map)(MockParser.AsStdFunction())(&tspan)),
              Not(IsOk()));
}

} // namespace
} // namespace formula
} // namespace latis
