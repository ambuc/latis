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

#include "src/graph/graph.h"

#include "src/test_utils/test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace latis {
namespace graph {
namespace {

using ::testing::_;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::Pointwise;
using ::testing::UnorderedElementsAre;

TEST(Graph, AddAndHas) {
  Graph<int> g;

  EXPECT_TRUE(g.AddEdge(0, 1));
  EXPECT_TRUE(g.HasEdge(0, 1));
}

TEST(Graph, DetectsCycle) {
  Graph<int> g;

  EXPECT_TRUE(g.AddEdge(0, 1));
  EXPECT_TRUE(g.HasEdge(0, 1));

  EXPECT_TRUE(g.AddEdge(1, 2));
  EXPECT_TRUE(g.HasEdge(1, 2));

  EXPECT_FALSE(g.AddEdge(2, 0));
  EXPECT_FALSE(g.HasEdge(2, 0));
}

TEST(Graph, Removal) {
  Graph<int> g;

  g.AddEdge(0, 1);
  EXPECT_TRUE(g.HasEdge(0, 1));
  EXPECT_THAT(g.GetDescendantsOf(0), UnorderedElementsAre(1));
  EXPECT_THAT(g.GetParentsOf(1), UnorderedElementsAre(0));

  g.RemoveEdge(0, 1);
  EXPECT_FALSE(g.HasEdge(0, 1));
  EXPECT_THAT(g.GetDescendantsOf(0), IsEmpty());
  EXPECT_THAT(g.GetParentsOf(1), IsEmpty());
}

class GraphTest : public ::testing::Test {
public:
  void SetUp() {
    // 0--> 1--> 2--> 3
    //      |
    //      v
    //      4
    g_.AddEdge(0, 1);
    g_.AddEdge(1, 2);
    g_.AddEdge(2, 3);
    g_.AddEdge(1, 4);
  }

protected:
  Graph<int> g_;
};

TEST_F(GraphTest, GetDescendantsOf) {
  EXPECT_THAT(g_.GetDescendantsOf(2), UnorderedElementsAre(3));
  EXPECT_THAT(g_.GetDescendantsOf(1), UnorderedElementsAre(2, 3, 4));

  // But we also care about toposort order. So GetDescendantsOf(1) must be
  // either [2,4,3] or [4,2,3], but either way 3 must be last.
  EXPECT_THAT(g_.GetDescendantsOf(1).back(), Eq(3));

  // By the same token:
  EXPECT_THAT(g_.GetDescendantsOf(0).front(), Eq(1));
  EXPECT_THAT(g_.GetDescendantsOf(0).back(), Eq(3));
}

TEST_F(GraphTest, GetParentsOf) {
  EXPECT_THAT(g_.GetParentsOf(1), UnorderedElementsAre(0));
  EXPECT_THAT(g_.GetParentsOf(2), UnorderedElementsAre(1));
  EXPECT_THAT(g_.GetParentsOf(3), UnorderedElementsAre(2));
  EXPECT_THAT(g_.GetParentsOf(4), UnorderedElementsAre(1));
}

TEST_F(GraphTest, GetDescendantsOfWithRemoval) {
  EXPECT_EQ(g_.GetDescendantsOf(0).front(), 1);
  EXPECT_EQ(g_.GetDescendantsOf(0).back(), 3);

  // 0--> 1         3
  //      |
  //      v
  //      4
  g_.RemoveEdge(1, 2);
  g_.RemoveEdge(2, 3);
  EXPECT_THAT(g_.GetDescendantsOf(0), ElementsAre(1, 4));

  // 0--> 1
  //      |
  //      v
  //      4--> 2--> 3
  g_.AddEdge(4, 2);
  g_.AddEdge(2, 3);
  EXPECT_THAT(g_.GetDescendantsOf(0), ElementsAre(1, 4, 2, 3));
}

TEST_F(GraphTest, TransactionSucceeds) {
  auto t = g_.NewTransaction();
  t->StageEdge(4, 3);
  t->StageEdge(4, 2);
  EXPECT_TRUE(t->Confirm());

  EXPECT_TRUE(g_.HasEdge(4, 3));
  EXPECT_TRUE(g_.HasEdge(4, 2));
}

TEST_F(GraphTest, TransactionFails) {
  {
    auto t = g_.NewTransaction();
    t->StageEdge(4, 3);
    t->StageEdge(4, 0); // would cause cycle;
    EXPECT_FALSE(t->Confirm());
  }
  EXPECT_FALSE(g_.HasEdge(4, 3));
  EXPECT_FALSE(g_.HasEdge(4, 0));
}

} // namespace
} // namespace graph
} // namespace latis
