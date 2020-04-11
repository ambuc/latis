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

#ifndef SRC_DISPLAY_UTILS_H_
#define SRC_DISPLAY_UTILS_H_

#include "proto/latis_msg.pb.h"
#include "src/xy.h"

#include <iostream>

#include "absl/strings/str_format.h"

namespace latis {

struct AmountFormatOptions {
  absl::ParsedFormat<'d'> int_format_options{"%2d"};
  absl::ParsedFormat<'f'> double_format_options{"%5.2f"};
};

std::string PrintCell(const Cell &cell, const AmountFormatOptions &afo);
std::string PrintCell(const Cell &cell) {
  return PrintCell(cell, AmountFormatOptions());
}
std::string PrintAmount(const Amount &amount, const AmountFormatOptions &afo);
std::string PrintAmount(const Amount &amount) {
  return PrintAmount(amount, AmountFormatOptions());
}

// Short-lived class to assist with stdout grid printing. Must not outlive its
// pointees.
//
// Usage:
//   auto gv = GridView({.height=10,.width=10});
//   gv.Write(xy, &cell); ...
//   std::cout << gv << std::endl;
//
class GridView {
public:
  struct Options {
    // Size
    int height = 0;
    int width = 0;

    int offset_x = 0;
    int offset_y = 0;

    // Formatting
    int floating_point_precision = 2;
  };

  explicit GridView(Options options)
      : height_(options.height), width_(options.width),
        offset_x_(options.offset_x), offset_y_(options.offset_y),
        floating_point_precision_(options.floating_point_precision),
        // initialized vector of vectors
        cells_(height_, std::vector<const Cell *>(width_, nullptr)),
        strings_(height_, std::vector<absl::optional<std::string>>(
                              width_, absl::nullopt)),
        widths_(width_, 0) {}

  void Write(XY xy, const Cell *cell_ptr);

  friend std::ostream &operator<<(std::ostream &os, const GridView &gv);

private:
  void PutHline(std::ostream &os) const;

  const int height_;
  const int width_;
  const int offset_x_;
  const int offset_y_;
  const int floating_point_precision_;

  std::vector<std::vector<const Cell *>> cells_;                  // y then x
  std::vector<std::vector<absl::optional<std::string>>> strings_; // y then x
  std::vector<int> widths_;
};

std::ostream &operator<<(std::ostream &os, const GridView &gv);

} // namespace latis

#endif // SRC_DISPLAY_UTILS_H_
