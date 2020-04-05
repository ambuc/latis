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
#include <iostream>
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
    bool result = !IsCycle(from, to) && edges_[from].insert(to).second;
    return result;
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

  // Returns a vector of nodes which are direct parents of some input node.
  std::vector<T> GetParentsOf(T node) {
    std::vector<T> vec{};
    for (const auto &[k, vs] : edges_) {
      if (vs.contains(node)) {
        vec.push_back(k);
      }
    }
    return vec;
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

  // void Print() {
  //   std::cout << "map: ";
  //   for (const auto &[k, vs] : edges_) {
  //     std::cout << k << ": [";
  //     for (const auto &v : vs) {
  //       std::cout << v << ",";
  //     }
  //     std::cout << "]; ";
  //   }
  //   std::cout << std::endl;
  // }

  absl::flat_hash_map<T, absl::flat_hash_set<T>> edges_;
};

} // namespace graph
} // namespace latis

#endif // SRC_GRAPH_GRAPH_H_
