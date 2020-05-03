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

#include "src/ui/color.h"

#include <ncurses.h>

namespace latis {
namespace ui {

void InitColors() {
  start_color();
  init_color(Color::ALICEBLUE, 240, 248, 255);
  init_color(Color::ANTIQUEWHITE, 250, 235, 215);
  init_color(Color::AQUA, 0, 255, 255);
  init_color(Color::AQUAMARINE, 127, 255, 212);
  init_color(Color::AZURE, 240, 255, 255);
  init_color(Color::BEIGE, 245, 245, 220);
  init_color(Color::BISQUE, 255, 228, 196);
  init_color(Color::BLACK, 0, 0, 0);
  init_color(Color::BLANCHEDALMOND, 255, 235, 205);
  init_color(Color::BLUE, 0, 0, 255);
  init_color(Color::BLUEVIOLET, 138, 43, 226);
  init_color(Color::BROWN, 165, 42, 42);
  init_color(Color::BURLYWOOD, 222, 184, 135);
  init_color(Color::CADETBLUE, 95, 158, 160);
  init_color(Color::CHARTREUSE, 127, 255, 0);
  init_color(Color::CHOCOLATE, 210, 105, 30);
  init_color(Color::CORAL, 255, 127, 80);
  init_color(Color::CORNFLOWERBLUE, 100, 149, 237);
  init_color(Color::CORNSILK, 255, 248, 220);
  init_color(Color::CRIMSON, 220, 20, 60);
  init_color(Color::CYAN, 0, 255, 255);
  init_color(Color::DARKBLUE, 0, 0, 139);
  init_color(Color::DARKCYAN, 0, 139, 139);
  init_color(Color::DARKGOLDENROD, 184, 134, 11);
  init_color(Color::DARKGRAY, 169, 169, 169);
  init_color(Color::DARKGREEN, 0, 100, 0);
  init_color(Color::DARKKHAKI, 189, 183, 107);
  init_color(Color::DARKMAGENTA, 139, 0, 139);
  init_color(Color::DARKOLIVEGREEN, 85, 107, 47);
  init_color(Color::DARKORANGE, 255, 140, 0);
  init_color(Color::DARKORCHID, 153, 50, 204);
  init_color(Color::DARKRED, 139, 0, 0);
  init_color(Color::DARKSALMON, 233, 150, 122);
  init_color(Color::DARKSEAGREEN, 143, 188, 139);
  init_color(Color::DARKSLATEBLUE, 72, 61, 139);
  init_color(Color::DARKSLATEGRAY, 47, 79, 79);
  init_color(Color::DARKTURQUOISE, 0, 206, 209);
  init_color(Color::DARKVIOLET, 148, 0, 211);
  init_color(Color::DEEPPINK, 255, 20, 147);
  init_color(Color::DEEPSKYBLUE, 0, 191, 255);
  init_color(Color::DIMGRAY, 105, 105, 105);
  init_color(Color::DODGERBLUE, 30, 144, 255);
  init_color(Color::FIREBRICK, 178, 34, 34);
  init_color(Color::FLORALWHITE, 255, 250, 240);
  init_color(Color::FORESTGREEN, 34, 139, 34);
  init_color(Color::FUCHSIA, 255, 0, 255);
  init_color(Color::GAINSBORO, 220, 220, 220);
  init_color(Color::GHOSTWHITE, 248, 248, 255);
  init_color(Color::GOLD, 255, 215, 0);
  init_color(Color::GOLDENROD, 218, 165, 32);
  init_color(Color::GRAY, 128, 128, 128);
  init_color(Color::GREEN, 0, 128, 0);
  init_color(Color::GREENYELLOW, 173, 255, 47);
  init_color(Color::HONEYDEW, 240, 255, 240);
  init_color(Color::HOTPINK, 255, 105, 180);
  init_color(Color::INDIANRED, 205, 92, 92);
  init_color(Color::INDIGO, 75, 0, 130);
  init_color(Color::IVORY, 255, 255, 240);
  init_color(Color::KHAKI, 240, 230, 140);
  init_color(Color::LAVENDER, 230, 230, 250);
  init_color(Color::LAVENDERBLUSH, 255, 240, 245);
  init_color(Color::LAWNGREEN, 124, 252, 0);
  init_color(Color::LEMONCHIFFON, 255, 250, 205);
  init_color(Color::LIGHTBLUE, 173, 216, 230);
  init_color(Color::LIGHTCORAL, 240, 128, 128);
  init_color(Color::LIGHTCYAN, 224, 255, 255);
  init_color(Color::LIGHTGOLDENRODYELLOW, 250, 250, 210);
  init_color(Color::LIGHTGRAY, 211, 211, 211);
  init_color(Color::LIGHTGREEN, 144, 238, 144);
  init_color(Color::LIGHTPINK, 255, 182, 193);
  init_color(Color::LIGHTSALMON, 255, 160, 122);
  init_color(Color::LIGHTSEAGREEN, 32, 178, 170);
  init_color(Color::LIGHTSKYBLUE, 135, 206, 250);
  init_color(Color::LIGHTSLATEGRAY, 119, 136, 153);
  init_color(Color::LIGHTSTEELBLUE, 176, 196, 222);
  init_color(Color::LIGHTYELLOW, 255, 255, 224);
  init_color(Color::LIME, 0, 255, 0);
  init_color(Color::LIMEGREEN, 50, 205, 50);
  init_color(Color::LINEN, 250, 240, 230);
  init_color(Color::MAGENTA, 255, 0, 255);
  init_color(Color::MAROON, 128, 0, 0);
  init_color(Color::MEDIUMAQUAMARINE, 102, 205, 170);
  init_color(Color::MEDIUMBLUE, 0, 0, 205);
  init_color(Color::MEDIUMORCHID, 186, 85, 211);
  init_color(Color::MEDIUMPURPLE, 147, 112, 219);
  init_color(Color::MEDIUMSEAGREEN, 60, 179, 113);
  init_color(Color::MEDIUMSLATEBLUE, 123, 104, 238);
  init_color(Color::MEDIUMSPRINGGREEN, 0, 250, 154);
  init_color(Color::MEDIUMTURQUOISE, 72, 209, 204);
  init_color(Color::MEDIUMVIOLETRED, 199, 21, 133);
  init_color(Color::MIDNIGHTBLUE, 25, 25, 112);
  init_color(Color::MINTCREAM, 245, 255, 250);
  init_color(Color::MISTYROSE, 255, 228, 225);
  init_color(Color::MOCCASIN, 255, 228, 181);
  init_color(Color::NAVAJOWHITE, 255, 222, 173);
  init_color(Color::NAVY, 0, 0, 128);
  init_color(Color::OLDLACE, 253, 245, 230);
  init_color(Color::OLIVE, 128, 128, 0);
  init_color(Color::OLIVEDRAB, 107, 142, 35);
  init_color(Color::ORANGE, 255, 165, 0);
  init_color(Color::ORANGERED, 255, 69, 0);
  init_color(Color::ORCHID, 218, 112, 214);
  init_color(Color::PALEGOLDENROD, 238, 232, 170);
  init_color(Color::PALEGREEN, 152, 251, 152);
  init_color(Color::PALETURQUOISE, 175, 238, 238);
  init_color(Color::PALEVIOLETRED, 219, 112, 147);
  init_color(Color::PAPAYAWHIP, 255, 239, 213);
  init_color(Color::PEACHPUFF, 255, 218, 185);
  init_color(Color::PERU, 205, 133, 63);
  init_color(Color::PINK, 255, 192, 203);
  init_color(Color::PLUM, 221, 160, 221);
  init_color(Color::POWDERBLUE, 176, 224, 230);
  init_color(Color::PURPLE, 128, 0, 128);
  init_color(Color::REBECCAPURPLE, 102, 51, 153);
  init_color(Color::RED, 255, 0, 0);
  init_color(Color::ROSYBROWN, 188, 143, 143);
  init_color(Color::ROYALBLUE, 65, 105, 225);
  init_color(Color::SADDLEBROWN, 139, 69, 19);
  init_color(Color::SALMON, 250, 128, 114);
  init_color(Color::SANDYBROWN, 244, 164, 96);
  init_color(Color::SEAGREEN, 46, 139, 87);
  init_color(Color::SEASHELL, 255, 245, 238);
  init_color(Color::SIENNA, 160, 82, 45);
  init_color(Color::SILVER, 192, 192, 192);
  init_color(Color::SKYBLUE, 135, 206, 235);
  init_color(Color::SLATEBLUE, 106, 90, 205);
  init_color(Color::SLATEGRAY, 112, 128, 144);
  init_color(Color::SNOW, 255, 250, 250);
  init_color(Color::SPRINGGREEN, 0, 255, 127);
  init_color(Color::STEELBLUE, 70, 130, 180);
  init_color(Color::TAN, 210, 180, 140);
  init_color(Color::TEAL, 0, 128, 128);
  init_color(Color::THISTLE, 216, 191, 216);
  init_color(Color::TOMATO, 255, 99, 71);
  init_color(Color::TURQUOISE, 64, 224, 208);
  init_color(Color::VIOLET, 238, 130, 238);
  init_color(Color::WHEAT, 245, 222, 179);
  init_color(Color::WHITE, 255, 255, 255);
  init_color(Color::WHITESMOKE, 245, 245, 245);
  init_color(Color::YELLOW, 255, 255, 0);
  init_color(Color::YELLOWGREEN, 154, 205, 50);
  for (int i = 0; i < Color::LAST; ++i) {
    init_pair(i, i, COLOR_BLACK);
  }
}

} // namespace ui
} // namespace latis
