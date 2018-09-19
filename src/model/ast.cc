// Copyright 2018 Justin Hu
//
// This file is part of the SM213 assembler.
//
// The SM213 assembler is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// The SM213 assembler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// the SM213 assembler.  If not, see <https://www.gnu.org/licenses/>.

#include "model/ast.h"

#include <algorithm>
#include <limits>
#include <map>
#include <tuple>

namespace sm213assembler::model {
namespace {
using std::all_of;
using std::find;
using std::get;
using std::map;
using std::max;
using std::numeric_limits;
using std::stoul;
using std::to_string;
using std::tuple;

struct Block {
  uint32_t startPos;
  vector<uint8_t> bytes;
};

bool validLabel(string s, bool expectColon = false) {
  return all_of(s.begin(), s.end() - (expectColon ? 1 : 0),
                [](char c) { return isalnum(c) || c == '_'; }) &&
         !isdigit(s.front()) && (!expectColon || s.back() == ':');
}

vector<uint8_t> bytesFromBlocks(vector<Block> blocks) noexcept {
  vector<uint8_t> result;
  uint32_t currPos;
  for (Block b : blocks) {  // generate code, keep placeholders
    result.resize(max(result.size(), static_cast<size_t>(b.startPos)));
    currPos = b.startPos;
    for (uint8_t byte : b.bytes) {
      if (currPos == result.size())
        result.push_back(byte);
      else
        result[currPos] = byte;
      currPos++;
    }
  }

  return result;
}

void replacePlaceholders(
    vector<uint8_t>& result, const map<string, uint32_t>& labelBinds,
    const map<uint32_t, tuple<string, unsigned, unsigned>>& labelUses) {
  for (auto iter : labelUses) {
    auto found = labelBinds.find(get<0>(iter.second));
    if (found == labelBinds.end()) {
      throw ParseError(get<1>(iter.second), get<2>(iter.second),
                       "unbound label '" + get<0>(iter.second) + "'.");
    }
    result[iter.first + 0] = static_cast<uint8_t>((found->second) >> (3 * 8));
    result[iter.first + 1] = static_cast<uint8_t>((found->second) >> (2 * 8));
    result[iter.first + 2] = static_cast<uint8_t>((found->second) >> (1 * 8));
    result[iter.first + 3] = static_cast<uint8_t>((found->second) >> (0 * 8));
  }
}
}  // namespace

ParseError::ParseError(unsigned l, unsigned c, string m) noexcept
    : msg{to_string(l) + ":" + to_string(c) + ":" + m} {}
const char* ParseError::what() const noexcept { return msg.c_str(); }

vector<uint8_t> generateBinary(const vector<Token>& tokens) {
  vector<Block> blocks;
  map<string, uint32_t> labelBinds;
  map<uint32_t, tuple<string, unsigned, unsigned>> labelUses;

  uint32_t currPos = 0;

  for (auto iter = tokens.cbegin(); iter != tokens.cend(); ++iter) {
    if (iter->value == "ld") {           // ld something
    } else if (iter->value == "st") {    // st something
    } else if (iter->value == "halt") {  // halt terminal
    } else if (iter->value == "nop") {   // nop terminal
    } else if (iter->value == "mov") {   // mov binop
    } else if (iter->value == "add") {   // add binop
    } else if (iter->value == "and") {   // and binop
    } else if (iter->value == "inc") {   // inc unop
    } else if (iter->value == "inca") {  // unca unop
    } else if (iter->value == "dec") {   // dec unop
    } else if (iter->value == "deca") {  // deca unop
    } else if (iter->value == "not") {   // not unop
    } else if (iter->value == "shl") {   // shl sh* form
    } else if (iter->value == "shr") {   // shr sh* form
    } else if (iter->value == ".pos") {  //.pos form
    } else if (iter->value == ".long" ||
               iter->value == ".data") {         // literal data
    } else if (validLabel(iter->value, true)) {  // label
    } else {
      throw ParseError(iter->lineNo, iter->charNo,
                       "unrecognized token '" + iter->value + "'.");
    }
  }

  vector<uint8_t> result = bytesFromBlocks(blocks);
  replacePlaceholders(result, labelBinds, labelUses);  // namespace

  return result;
}
}  // namespace sm213assembler::model
}
}  // namespace sm213assembler::model