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

#ifndef SRC_PARSER_PARSER_COMBINATORS_H_
#define SRC_PARSER_PARSER_COMBINATORS_H_

#include "src/formula/common.h"
#include "src/status_utils/status_macros.h"

#include "absl/types/optional.h"
#include "absl/types/variant.h"

namespace latis {
namespace formula {

// Helpful usings which make writing code in a header feel more natural.
using Status = ::google::protobuf::util::Status;

template <typename T> //
using StatusOr = ::google::protobuf::util::StatusOr<T>;

using TSpan = absl::Span<const Token>;

template <typename T> //
using Prsr = std::function<StatusOr<T>(TSpan *)>;

// Parser combinator |AnyVariant|.
//
// Useful for combining n parsers, all of whom have different return types.
//
// Usage:
//     AnyVariant<A, B>(ConsumeA, ConsumeB)(tspan);
//
// Types:
//     AnyVariant :: [Prsr<A>, Prsr<B>, ...]
//                -> Prsr<variant<A, B, ...>>
template <typename... O> //
static std::function<StatusOr<absl::variant<O...>>(TSpan *)>
AnyVariant(std::function<StatusOr<O>(TSpan *)>... ls) {
  return [&](TSpan * tspan) -> StatusOr<absl::variant<O...>> {
    std::vector<std::function<StatusOr<absl::variant<O...>>(TSpan *)>> fns{
        ls...};
    for (const auto fn : fns) {
      TSpan lcl = *tspan;
      if (const auto v = fn(&lcl); v.ok()) {
        *tspan = lcl;
        return v.ValueOrDie();
      }
    }
    return Status(::google::protobuf::util::error::INVALID_ARGUMENT,
                  "no match in AnyVariant<>().");
  };
}

// Parser combinator |Any|.
//
// Useful for combining n parsers, all of whom share a return type.
//
// Usage:
//     Any<A>(ConsumeA1, ConsumeA2)(tspan);
//
// Types:
//     Any :: [Prsr<T>]
//         -> Prsr<T>
template <typename T> //
static Prsr<T> Any(const std::vector<Prsr<T>> fns) {
  return [&](TSpan *tspan) -> StatusOr<T> {
    for (const auto fn : fns) {
      TSpan lcl = *tspan;
      if (const auto v = fn(&lcl); v.ok()) {
        *tspan = lcl;
        return v.ValueOrDie();
      }
    }
    return Status(::google::protobuf::util::error::INVALID_ARGUMENT,
                  "no match in Any<>()");
  };
}

// Parser combinator |Maybe|.
//
// Usage:
//     Maybe<A>(ConsumeA)(tspan);
//
// Types:
//     Maybe :: (TSpan* -> StatusOr<T>)
//           -> (TSpan* -> optional<T>)
template <typename T> //
static std::function<absl::optional<T>(TSpan *)> Maybe(const Prsr<T> &fn) {
  return [&](TSpan *tspan) -> absl::optional<T> {
    TSpan lcl = *tspan;

    if (const auto v = fn(&lcl); v.ok()) {
      *tspan = lcl;
      return v.ValueOrDie();
    }
    return absl::nullopt;
  };
}

// Parser combinator |WithRestriction|. Returns the value if the lambda returns
// true.
//
// Usage:
//     auto r = [](int i) -> bool { return i != 0; }
//     WithRestriction<int>(r)(ConsumeInt)(tspan);
//
// Types:
//     WithRestriction :: std::function<bool(T)>
//                     -> Prsr<T>
//                     -> Prsr<T>
template <typename T> //
static std::function<Prsr<T>(const Prsr<T> &)>
WithRestriction(std::function<bool(T)> r) {
  return [&r](const Prsr<T> &p) -> Prsr<T> {
    return [&r, &p](TSpan *tspan) -> StatusOr<T> {
      TSpan lcl = *tspan;
      if (const auto v = p(&lcl); !v.ok()) {
        return v.status();
      } else {
        if (!r(v.ValueOrDie())) {
          return Status(::google::protobuf::util::error::INVALID_ARGUMENT,
                        "Can't WithRestriction<>(): didn't pass restriction.");
        }
        *tspan = lcl;
        return v.ValueOrDie();
      }
    };
  };
}

// Parser combinator |InSequence|.
//
// Usage:
//    InSequence<std::tuple<A, B, C>>(ConsumeA, ConsumeB, ConsumeC)(tspan);
//
// Types:
//    InSequence :: Prsr<A>
//               -> Prsr<B>
//               -> Prsr<C>
//               -> (TSpan* -> StatusOr<std::tuple<A, B, C>>)

template <typename... Ts, std::size_t... I> //
static StatusOr<std::tuple<Ts...>>
InSequence_impl(TSpan *tspan, std::tuple<const Prsr<Ts> &...> fns_tuple,
                std::index_sequence<I...>) {
  TSpan lcl = *tspan;
  std::tuple<Ts...> resultant;

  auto loop_status = OkStatus();

  std::apply(
      [&loop_status, &resultant, &lcl](Prsr<Ts> const &... fn) {
        (
            [&loop_status, &resultant, &lcl, &fn] {
              if (loop_status.ok()) {
                if (const auto maybe_value = fn(&lcl); maybe_value.ok()) {
                  std::get<I>(resultant) = maybe_value.ValueOrDie();
                } else {
                  loop_status = maybe_value.status();
                }
              }
            }(),
            ...);
      },
      fns_tuple);

  RETURN_IF_ERROR_(loop_status);

  *tspan = lcl;
  return resultant;
}

template <typename... Ts> //
static std::function<StatusOr<std::tuple<Ts...>>(TSpan *)>
InSequence(const Prsr<Ts> &... fns) {
  return [&](TSpan * tspan) -> StatusOr<std::tuple<Ts...>> {

    TSpan lcl = *tspan;

    std::tuple<Ts...> resultant;
    ASSIGN_OR_RETURN_(resultant,
                      InSequence_impl(&lcl, std::forward_as_tuple(fns...),
                                      std::index_sequence_for<Ts...>{}));

    *tspan = lcl;
    return resultant;
  };
}

// Parser combinator |WithTransformation|
//
// Usage: (A -> B)
//     std::function<bool(int)> tr = [](int i) -> bool { return i == 0; };
//     WithTransformation<A, B>(tr)(ConsumeInt)(tspan);
//
// Types:
//   WithTransformation :: (A -> B)
//                      -> Prsr<A>
//                      -> Prsr<B>
template <typename A, typename B> //
static std::function<Prsr<B>(const Prsr<A> &)>
WithTransformation(std::function<B(A)> t) {
  return [&t](const Prsr<A> &p) -> Prsr<B> {
    return [&t, &p](TSpan *tspan) -> StatusOr<B> {
      TSpan lcl = *tspan;
      const auto v = p(&lcl);
      if (!v.ok()) {
        return v.status();
      }
      *tspan = lcl;
      return t(v.ValueOrDie());
    };
  };
}

// Parser combinator |WithLookup|
//
// Usage:
//     absl::flat_hash_map<K, V> key_to_val = {{"A", "a"}, {"B", "b"}};
//     WithLookup<K, V>(key_to_val)(ConsumeString)(tspan);
//     // If the output of the inner parser is in the map, returns the match.
//     // Otherwise returns a status explaining.
//     // If the output of the inner parser is a Status, returns that.
template <typename M> //
static std::function<
    Prsr<typename M::mapped_type>(const Prsr<typename M::key_type> &)>
WithLookup(const M &map) {
  return [&map](const Prsr<typename M::key_type> &p)
             -> Prsr<typename M::mapped_type> {
    return [&map, &p ](TSpan * tspan) -> StatusOr<typename M::mapped_type> {
      TSpan lcl = *tspan;

      const StatusOr<typename M::key_type> maybe_key = p(&lcl);
      if (!maybe_key.ok()) {
        return maybe_key.status();
      }

      typename M::key_type key = maybe_key.ValueOrDie();

      const typename M::const_iterator it = map.find(key);
      if (it == map.end()) {
        return Status(::google::protobuf::util::error::INVALID_ARGUMENT,
                      "no lookup match");
      }

      *tspan = lcl;
      return it->second;
    };
  };
}

//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//

} // namespace formula
} // namespace latis

#endif // SRC_PARSER_PARSER_COMBINATORS_H_
