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
#include "absl/memory/memory.h"
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
    if (IsCycle(from, to)) {
      return false;
    }
    p2c_[from].insert(to);
    c2p_[to].insert(from);
    return true;
  }

  // The inverse of AddEdge, except there is no checking of whether the edge
  // existed before.
  void RemoveEdge(T from, T to) {
    p2c_[from].erase(to);
    c2p_[to].erase(from);
  }

  // Possibly useful.
  bool HasEdge(T from, T to) { return p2c_[from].contains(to); }

  // Returns a vector of nodes descending from some input node.
  // The returned vector will be in topological order.
  std::vector<T> GetDescendantsOf(T node) {
    std::vector<T> vec{};
    GetDescendantsOfInternal(node, &vec);
    return vec;
  }

  // Returns a vector of nodes which are _direct_ parents of some input node.
  std::vector<T> GetParentsOf(T node) {
    return std::vector<T>(c2p_[node].begin(), c2p_[node].end());
  }

  // Represents an in-progress transaction.
  // RAII for potentially unwindable additions.
  // If transaction.Confirm() is not called, these insertons will be undone at
  // destruction..
  //
  // Example usage:
  //   Graph g;
  //   auto t = g.NewTransaction();
  //   t->StageEdge(f1,t1);
  //   t->StageEdge(f2,t2); ...
  //   bool is_valid = t->Confirm();
  //
  class Transaction {
  public:
    Transaction(Graph *g) : g_(g) {}
    ~Transaction() {
      if (!is_valid_) {
        for (const auto &[f, t] : inserted_) {
          g_->RemoveEdge(f, t);
        }
      }
    }
    void StageEdge(T from, T to) {
      if (!is_valid_) {
        return;
      }
      if (g_->AddEdge(from, to)) {
        inserted_.push_back({from, to});
      } else {
        is_valid_ = false;
      }
    }
    bool Confirm() { return is_valid_; }

  private:
    Graph *g_;
    std::vector<std::pair<T, T>> inserted_;
    bool is_valid_{true};
  };

  std::unique_ptr<Transaction> NewTransaction() {
    return absl::make_unique<Transaction>(this);
  }

private:
  // Returns true if a cycle is found.
  bool IsCycle(T cand, T node) {
    return cand == node || std::any_of(p2c_[node].begin(), p2c_[node].end(),
                                       [&](const T &child) {
                                         return (child == cand) ||
                                                IsCycle(cand, child);
                                       });
  }

  void GetDescendantsOfInternal(T node, std::vector<T> *output) {
    for (const T &child : p2c_[node]) {
      output->push_back(child);
    }
    for (const T &child : p2c_[node]) {
      GetDescendantsOfInternal(child, output);
    }
  }

  absl::flat_hash_map<T, absl::flat_hash_set<T>> p2c_;
  absl::flat_hash_map<T, absl::flat_hash_set<T>> c2p_;
};

} // namespace graph
} // namespace latis

#endif // SRC_GRAPH_GRAPH_H_
