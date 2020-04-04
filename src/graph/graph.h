/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_GRAPH_GRAPH_H_
#define SRC_GRAPH_GRAPH_H_

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include <algorithm>
#include <vector>

namespace latis {
namespace graph {

// Implements online dynamic topological sort.
//
// For a set of nodes of type T, maintains a directed acyclic graph of edges
// between nodes.
template <typename T> //
class Graph {
public:
  Graph() {}

  // Inserts an edge between two nodes.
  // If an edge creates a cycle, this method will return false and not perform
  // the insertion. Otherwise, will return true.
  bool AddEdge(T from, T to) {
    return !IsCycle(from, to) && edges_[from].insert(to).second;
  }
  void RemoveEdge(T from, T to) { edges_[from].erase(to); }

  bool HasEdge(T from, T to) { return edges_[from].contains(to); }

  // Returns a vector of nodes descending from some input node.
  // The returned vector will be in topological order.
  std::vector<T> GetDescendantsOf(T node) {
    std::vector<T> vec{};
    GetDescendantsOfInternal(node, &vec);
    return vec;
  }

  // Removes a node and all edges starting from, or ending at, that node.
  void Remove(T node) {
    edges_.erase(node);
    for (auto &[k, v] : edges_) {
      v.erase(node);
    }
  }

private:
  // Returns true if a cycle is found.
  bool IsCycle(T cand, T node) {
    auto it = edges_[node];
    return std::any_of(it.begin(), it.end(),
                       [this, &cand](const T &child) -> bool {
                         return (child == cand) || IsCycle(cand, child);
                       });
  }

  void GetDescendantsOfInternal(T node, std::vector<T> *output) {
    for (const T &child : edges_[node]) {
      output->push_back(child);
    }
    for (const T &child : edges_[node]) {
      GetDescendantsOfInternal(child, output);
    }
  }

  absl::flat_hash_map<T, absl::flat_hash_set<T>> edges_;
};

} // namespace graph
} // namespace latis

#endif // SRC_GRAPH_GRAPH_H_
