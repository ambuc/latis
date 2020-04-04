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

using ::testing::ElementsAre;
using ::testing::Eq;
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

TEST(Graph, GetDescendantsOf) {
  Graph<int> g;

  // 0--> 1--> 2--> 3
  //      |
  //      v
  //      4
  EXPECT_TRUE(g.AddEdge(0, 1));
  EXPECT_TRUE(g.AddEdge(1, 2));
  EXPECT_TRUE(g.AddEdge(2, 3));
  EXPECT_TRUE(g.AddEdge(1, 4));

  EXPECT_THAT(g.GetDescendantsOf(2), UnorderedElementsAre(3));
  EXPECT_THAT(g.GetDescendantsOf(1), UnorderedElementsAre(2, 3, 4));

  // But we also care about toposort order. So GetDescendantsOf(1) must be
  // either [2,4,3] or [4,2,3], but either way 3 must be last.
  EXPECT_THAT(g.GetDescendantsOf(1).back(), Eq(3));

  // By the same token:
  EXPECT_THAT(g.GetDescendantsOf(0).front(), Eq(1));
  EXPECT_THAT(g.GetDescendantsOf(0).back(), Eq(3));
}

TEST(Graph, RemovalOfParent) {
  Graph<int> g;

  g.AddEdge(0, 1);
  EXPECT_TRUE(g.HasEdge(0, 1));

  g.Remove(0);
  EXPECT_FALSE(g.HasEdge(0, 1));
}

TEST(Graph, RemovalOfChild) {
  Graph<int> g;

  g.AddEdge(0, 1);
  EXPECT_TRUE(g.HasEdge(0, 1));

  g.Remove(1);
  EXPECT_FALSE(g.HasEdge(0, 1));
}

TEST(Graph, GetDescendantsOfWithRemoval) {
  Graph<int> g;
  // 0--> 1--> 2--> 3
  //      |
  //      v
  //      4
  g.AddEdge(0, 1);
  g.AddEdge(1, 2);
  g.AddEdge(2, 3);
  g.AddEdge(1, 4);

  {
    auto desc_0 = g.GetDescendantsOf(0);
    EXPECT_EQ(desc_0.front(), 1);
    EXPECT_EQ(desc_0.back(), 3);
  }

  // 0--> 1         3
  //      |
  //      v
  //      4
  g.Remove(2);
  EXPECT_THAT(g.GetDescendantsOf(0), ElementsAre(1, 4));

  // 0--> 1
  //      |
  //      v
  //      4--> 2--> 3
  g.AddEdge(4, 2);
  g.AddEdge(2, 3);
  EXPECT_THAT(g.GetDescendantsOf(0), ElementsAre(1, 4, 2, 3));
}

} // namespace
} // namespace graph
} // namespace latis
