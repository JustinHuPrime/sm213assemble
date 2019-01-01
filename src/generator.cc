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

#include "generator.h"
#include "util.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <tuple>

namespace sm213assemble::model {
namespace {
using sm213assemble::util::hexify;
using std::all_of;
using std::cerr;
using std::find;
using std::get;
using std::invalid_argument;
using std::list;
using std::map;
using std::max;
using std::numeric_limits;
using std::pair;
using std::stol;
using std::stoul;
using std::to_string;
using std::tuple;

struct Block {
  uint32_t startPos;
  vector<uint8_t> bytes;
};

struct LabelUse {
  uint32_t useLocn;
  bool isPCRel;
  string labelName;
  unsigned labelLine;
  unsigned labelChar;

  LabelUse(uint32_t useLocn, string labelName, unsigned lineNo, unsigned charNo,
           bool isPCRel) noexcept;
};

LabelUse::LabelUse(uint32_t ul, string ln, unsigned ll, unsigned lc,
                   bool pcr) noexcept
    : useLocn{ul}, isPCRel{pcr}, labelName{ln}, labelLine{ll}, labelChar{lc} {}

typedef vector<Token>::const_iterator const_iter;

bool validLabel(string s, bool expectColon = false) {
  return all_of(s.begin(), s.end() - (expectColon ? 1 : 0),
                [](char c) { return isalnum(c) || c == '_'; }) &&
         !isdigit(s.front()) && (!expectColon || s.back() == ':');
}

vector<uint8_t> bytesFromBlocks(vector<Block> blocks) noexcept {
  vector<uint8_t> result;

  vector<pair<uint32_t, uint32_t>> areas;
  for (const Block& b : blocks) {  // check for collisions
    for (const auto& p : areas) {
      if ((p.first <= b.startPos && b.startPos < p.second) ||
          (p.first < b.startPos + b.bytes.size() &&
           b.startPos + b.bytes.size() <= p.second) ||
          (p.first > b.startPos && b.startPos + b.bytes.size() >= p.second))
        cerr << "Warning: overwriting some bytes in block from " << std::hex
             << p.first << " to " << p.second << std::dec << ".\n";
    }
    areas.push_back(
        pair<uint32_t, uint32_t>(b.startPos, b.startPos + b.bytes.size()));
  }

  size_t maxNeeded = 0;
  for (const Block& b : blocks) {
    maxNeeded = max(maxNeeded, b.startPos + b.bytes.size());
  }

  result.resize(maxNeeded);

  for (const Block& b : blocks) {  // generate code, keep placeholders
    uint32_t currPos = b.startPos;
    for (uint8_t byte : b.bytes) {
      result[currPos] = byte;
      currPos++;
    }
  }

  return result;
}
void replacePlaceholders(vector<uint8_t>& result,
                         const map<string, uint32_t>& labelBinds,
                         const list<LabelUse>& labelUses) {
  for (const auto& iter : labelUses) {
    auto found = labelBinds.find(iter.labelName);
    if (found == labelBinds.end()) {
      throw ParseError(iter.labelLine, iter.labelChar,
                       "unbound label '" + iter.labelName + "'.");
    } else if (iter.isPCRel) {
      long diff = static_cast<long>(iter.useLocn) + 1 -
                  static_cast<long>(found->second);
      if (diff % 2 != 0)
        throw ParseError(
            iter.labelLine, iter.labelChar,
            "Cannot have label offset not divisible by two, currently " +
                hexify(diff) + ".");
      diff /= 2;
      if (diff > 0x7f || diff < -0x80)
        throw ParseError(
            iter.labelLine, iter.labelChar,
            "use of label '" + iter.labelName +
                "' may not be more than 0x80 from its binding, currently " +
                hexify(2 * diff) + ".");
      result[iter.useLocn] = static_cast<uint8_t>(static_cast<int8_t>(diff));
    } else {
      result[iter.useLocn + 0] =
          static_cast<uint8_t>((found->second) >> (3 * 8));
      result[iter.useLocn + 1] =
          static_cast<uint8_t>((found->second) >> (2 * 8));
      result[iter.useLocn + 2] =
          static_cast<uint8_t>((found->second) >> (1 * 8));
      result[iter.useLocn + 3] =
          static_cast<uint8_t>((found->second) >> (0 * 8));
    }
  }
}

[[noreturn]] void badToken(const const_iter& iter) {
  if (iter->value != "\n")
    throw ParseError(iter->lineNo, iter->charNo,
                     "unrecognized token '" + iter->value + "'.");
  else
    throw ParseError(iter->lineNo, iter->charNo, "unexpected newline.");
}

void requireNext(const const_iter& iter,
                 const vector<Token>::const_iterator& end) {
  if (iter + 1 == end) {
    throw ParseError(
        iter->lineNo, iter->charNo,
        "expected token after '" + iter->value + "', but reached end of file.");
  }
}
void expect(const const_iter& iter, const string& expected,
            string expectedMsg = "") {
  if (expectedMsg == "") expectedMsg = expected;
  if (iter->value != expected) {
    throw ParseError(
        iter->lineNo, iter->charNo,
        "expected '" + expectedMsg + "', but got '" + iter->value + "'.");
  }
}

unsigned long getNumber(const const_iter& iter) {
  size_t eidx;
  unsigned long buffer;
  try {
    buffer = stoul(iter->value, &eidx, 0);
  } catch (const invalid_argument&) {
    throw ParseError(
        iter->lineNo, iter->charNo,
        "expected unsigned number, but got '" + iter->value + "'.");
  }
  if (eidx != iter->value.length()) {
    badToken(iter);
  } else {
    return buffer;
  }
}
long getNumberSigned(const const_iter& iter) {
  size_t eidx;
  long buffer;
  try {
    buffer = stol(iter->value, &eidx, 0);
  } catch (const invalid_argument&) {
    throw ParseError(iter->lineNo, iter->charNo,
                     "expected sighed number, but got '" + iter->value + "'.");
  }
  if (eidx != iter->value.length()) {
    badToken(iter);
  } else {
    return buffer;
  }
}
void addInt(uint32_t number, Block& b) {
  b.bytes.push_back(static_cast<uint8_t>(number >> (3 * 8)));
  b.bytes.push_back(static_cast<uint8_t>(number >> (2 * 8)));
  b.bytes.push_back(static_cast<uint8_t>(number >> (1 * 8)));
  b.bytes.push_back(static_cast<uint8_t>(number >> (0 * 8)));
}
uint32_t getInt(const const_iter& iter) {
  unsigned long buffer = getNumber(iter);
  if (buffer > numeric_limits<uint32_t>().max())
    throw ParseError(iter->lineNo, iter->charNo,
                     "out of range: " + iter->value + " must fit in 4 bytes.");
  return static_cast<uint32_t>(buffer);
}
uint8_t getOneReg(const_iter& iter) {
  if (iter->value.length() != 2 || iter->value[0] != 'r' ||
      (iter->value[1] < '0' || iter->value[1] > '7'))
    throw ParseError(iter->lineNo, iter->charNo,
                     "Expected r[0-7], got '" + iter->value + "'.");
  return iter->value[1] - '0';
}
uint8_t getTwoRegs(const_iter& iter, const const_iter& end) {
  uint8_t acc = getOneReg(iter);
  requireNext(iter, end);
  ++iter;
  expect(iter, ",");
  requireNext(iter, end);
  ++iter;
  acc = (acc << 4) | getOneReg(iter);
  return acc;
}

}  // namespace

ParseError::ParseError(unsigned l, unsigned c, string m) noexcept
    : msg{to_string(l) + ":" + to_string(c) + ":" + m} {}
const char* ParseError::what() const noexcept { return msg.c_str(); }

vector<uint8_t> generateBinary(const vector<Token>& tokens) {
  vector<Block> blocks;
  map<string, uint32_t> labelBinds;
  list<LabelUse> labelUses;
  // tuple<address, difference?, tuple<labelName, useLine, useColumn>>

  uint32_t currPos = 0;
  Block currBlock;
  currBlock.startPos = 0;

  for (auto iter = tokens.cbegin(); iter != tokens.cend(); ++iter) {
    if (iter->value == "ld") {  // ld something
      requireNext(iter, tokens.cend());
      ++iter;
      if (iter->value == "$") {  // label or literal
        uint32_t address;
        requireNext(iter, tokens.cend());
        currPos += 2;
        ++iter;
        if (validLabel(iter->value)) {
          address = 0x5a5a5a5a;  // magic number - 0x5--- is an invalid opcode
          labelUses.push_back(LabelUse(currPos, iter->value, iter->lineNo,
                                       iter->charNo, false));
        } else {
          address = getInt(iter);
        }
        requireNext(iter, tokens.cend());
        ++iter;
        expect(iter, ",");
        requireNext(iter, tokens.cend());
        ++iter;
        currBlock.bytes.push_back(getOneReg(iter));
        currBlock.bytes.push_back(0x0);
        addInt(address, currBlock);
        currPos += 4;
      } else if (iter->value == "(") {  // (rn) or (rb, ri, 4) forms
        requireNext(iter, tokens.cend());
        ++iter;
        uint8_t temp = getOneReg(iter);
        requireNext(iter, tokens.cend());
        ++iter;
        if (iter->value == ")") {  // (rn) form
          uint8_t from = temp;
          currBlock.bytes.push_back(0x10);
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, ",");
          requireNext(iter, tokens.cend());
          ++iter;
          currBlock.bytes.push_back((from << 4) | getOneReg(iter));
        } else {  // (rs, ri, 4) form
          // 2sid
          uint8_t base = temp;
          expect(iter, ",", ",' or '(");
          requireNext(iter, tokens.cend());
          ++iter;
          uint8_t index = getOneReg(iter);
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, ",");
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, "4");
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, ")");
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, ",");
          requireNext(iter, tokens.cend());
          ++iter;
          uint8_t dest = getOneReg(iter);
          currBlock.bytes.push_back(0x20 | base);
          currBlock.bytes.push_back((index << 4) | dest);
        }
        currPos += 2;
      } else {  // o(rn) form
        unsigned long buffer = getNumber(iter);
        if (buffer % 4 != 0)
          throw ParseError(iter->lineNo, iter->charNo,
                           iter->value + " must be divisible by four.");
        if (buffer / 4 > 0xf)
          throw ParseError(iter->lineNo, iter->charNo,
                           "out of range: a quarter of " + iter->value +
                               " must fit in 1 nibble.");
        requireNext(iter, tokens.cend());
        ++iter;
        expect(iter, "(");
        requireNext(iter, tokens.cend());
        ++iter;
        uint8_t from = getOneReg(iter);
        requireNext(iter, tokens.cend());
        ++iter;
        expect(iter, ")");
        requireNext(iter, tokens.cend());
        ++iter;
        expect(iter, ",");
        requireNext(iter, tokens.cend());
        ++iter;
        uint8_t to = getOneReg(iter);
        // 1psd
        currBlock.bytes.push_back(0x10 | static_cast<uint8_t>(buffer / 4));
        currBlock.bytes.push_back((from << 4) | to);
        currPos += 2;
      }
    } else if (iter->value == "st") {  // st something
      requireNext(iter, tokens.cend());
      ++iter;
      uint8_t from = getOneReg(iter);
      requireNext(iter, tokens.cend());
      ++iter;
      expect(iter, ",");
      requireNext(iter, tokens.cend());
      ++iter;
      if (iter->value == "(") {  // (rn) or (rb, ri, 4) forms
        requireNext(iter, tokens.cend());
        ++iter;
        uint8_t temp = getOneReg(iter);
        requireNext(iter, tokens.cend());
        ++iter;
        if (iter->value == ")") {  // (rn) form
          uint8_t to = temp;
          currBlock.bytes.push_back(0x30 | from);
          currBlock.bytes.push_back(to);
        } else {  // (rd, ri, 4) form
          // 4sdi
          uint8_t base = temp;
          expect(iter, ",", ",' or '(");
          requireNext(iter, tokens.cend());
          ++iter;
          uint8_t index = getOneReg(iter);
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, ",");
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, "4");
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, ")");
          currBlock.bytes.push_back(0x40 | from);
          currBlock.bytes.push_back((base << 4) | index);
        }
      } else {  // o(rn) form
        unsigned long buffer = getNumber(iter);
        if (buffer % 4 != 0)
          throw ParseError(iter->lineNo, iter->charNo,
                           iter->value + " must be divisible by four.");
        if (buffer / 4 > 0xf)
          throw ParseError(iter->lineNo, iter->charNo,
                           "out of range: a quarter of " + iter->value +
                               " must fit in 1 nibble.");
        requireNext(iter, tokens.cend());
        ++iter;
        expect(iter, "(");
        requireNext(iter, tokens.cend());
        ++iter;
        uint8_t to = getOneReg(iter);
        requireNext(iter, tokens.cend());
        ++iter;
        expect(iter, ")");
        // 3spd
        currBlock.bytes.push_back(0x30 | from);
        currBlock.bytes.push_back(((static_cast<uint8_t>(buffer / 4)) << 4) |
                                  to);
      }
      currPos += 2;
    } else if (iter->value == "halt") {  // halt terminal
      currBlock.bytes.push_back(0xf0);
      currBlock.bytes.push_back(0x00);
      currPos += 2;
    } else if (iter->value == "nop") {  // nop terminal
      currBlock.bytes.push_back(0xff);
      currBlock.bytes.push_back(0x00);
      currPos += 2;
    } else if (iter->value == "mov") {  // mov binop
      currBlock.bytes.push_back(0x60);
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(getTwoRegs(iter, tokens.cend()));
      currPos += 2;
    } else if (iter->value == "add") {  // add binop
      currBlock.bytes.push_back(0x61);
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(getTwoRegs(iter, tokens.cend()));
      currPos += 2;
    } else if (iter->value == "and") {  // and binop
      currBlock.bytes.push_back(0x62);
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(getTwoRegs(iter, tokens.cend()));
      currPos += 2;
    } else if (iter->value == "inc") {  // inc unop
      currBlock.bytes.push_back(0x63);
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(getOneReg(iter));
      currPos += 2;
    } else if (iter->value == "inca") {  // unca unop
      currBlock.bytes.push_back(0x64);
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(getOneReg(iter));
      currPos += 2;
    } else if (iter->value == "dec") {  // dec unop
      currBlock.bytes.push_back(0x65);
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(getOneReg(iter));
      currPos += 2;
    } else if (iter->value == "deca") {  // deca unop
      currBlock.bytes.push_back(0x66);
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(getOneReg(iter));
      currPos += 2;
    } else if (iter->value == "not") {  // not unop
      currBlock.bytes.push_back(0x67);
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(getOneReg(iter));
      currPos += 2;
    } else if (iter->value == "shl") {  // shl sh* form
      requireNext(iter, tokens.cend());
      ++iter;
      expect(iter, "$");
      requireNext(iter, tokens.cend());
      ++iter;
      unsigned long buffer = getNumber(iter);
      if (buffer > 0x7f) {
        throw ParseError(
            iter->lineNo, iter->charNo,
            "out of range: " + iter->value + " must fit in 1 byte.");
      }
      requireNext(iter, tokens.cend());
      ++iter;
      expect(iter, ",");
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(0x70 | getOneReg(iter));
      currBlock.bytes.push_back(static_cast<uint8_t>(buffer));
      currPos += 2;
    } else if (iter->value == "shr") {  // shr sh* form
      requireNext(iter, tokens.cend());
      ++iter;
      expect(iter, "$");
      requireNext(iter, tokens.cend());
      ++iter;
      unsigned long buffer = getNumber(iter);
      if (buffer > 0x80) {
        throw ParseError(
            iter->lineNo, iter->charNo,
            "out of range: " + iter->value + " must fit in 1 byte.");
      }
      requireNext(iter, tokens.cend());
      ++iter;
      expect(iter, ",");
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(0x70 | getOneReg(iter));
      currBlock.bytes.push_back(static_cast<uint8_t>(-buffer));
      currPos += 2;
    } else if (iter->value == "br") {  // br form
      requireNext(iter, tokens.cend());
      ++iter;
      if (validLabel(iter->value)) {
        currBlock.bytes.push_back(0x80);
        currBlock.bytes.push_back(0x5a);
        labelUses.push_back(LabelUse(currPos + 1, iter->value, iter->lineNo,
                                     iter->charNo, true));
      } else {
        long buffer = getNumberSigned(iter);
        if (buffer % 2 != 0) {
          throw ParseError(iter->lineNo, iter->charNo,
                           iter->value + " must be divisible by two.");
        } else if (buffer / 2 > 0x7f || buffer / 2 < -0x80) {
          throw ParseError(
              iter->lineNo, iter->charNo,
              "out of range: half of " + iter->value + " must fit in 1 byte.");
        }
        currBlock.bytes.push_back(0x80);
        currBlock.bytes.push_back(
            static_cast<uint8_t>(static_cast<int8_t>(buffer / 2)));
      }
      currPos += 2;
    } else if (iter->value == "beq") {  // beq form
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(0x90 | getOneReg(iter));
      requireNext(iter, tokens.cend());
      ++iter;
      expect(iter, ",");
      requireNext(iter, tokens.cend());
      ++iter;
      if (validLabel(iter->value)) {
        currBlock.bytes.push_back(0x5a);
        labelUses.push_back(LabelUse(currPos + 1, iter->value, iter->lineNo,
                                     iter->charNo, true));
      } else {
        long buffer = getNumberSigned(iter);
        if (buffer % 2 != 0) {
          throw ParseError(iter->lineNo, iter->charNo,
                           iter->value + " must be divisible by two.");
        } else if (buffer / 2 > 0x7f || buffer / 2 < -0x80) {
          throw ParseError(
              iter->lineNo, iter->charNo,
              "out of range: half of " + iter->value + " must fit in 1 byte.");
        }
        currBlock.bytes.push_back(
            static_cast<uint8_t>(static_cast<int8_t>(buffer / 2)));
      }
      currPos += 2;
    } else if (iter->value == "bgt") {  // bgt form
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(0xa0 | getOneReg(iter));
      requireNext(iter, tokens.cend());
      ++iter;
      expect(iter, ",");
      requireNext(iter, tokens.cend());
      ++iter;
      if (validLabel(iter->value)) {
        currBlock.bytes.push_back(0x5a);
        labelUses.push_back(LabelUse(currPos + 1, iter->value, iter->lineNo,
                                     iter->charNo, true));
      } else {
        long buffer = getNumberSigned(iter);
        if (buffer % 2 != 0) {
          throw ParseError(iter->lineNo, iter->charNo,
                           iter->value + " must be divisible by two.");
        } else if (buffer / 2 > 0x7f || buffer / 2 < -0x80) {
          throw ParseError(
              iter->lineNo, iter->charNo,
              "out of range: half of " + iter->value + " must fit in 1 byte.");
        }
        currBlock.bytes.push_back(
            static_cast<uint8_t>(static_cast<int8_t>(buffer / 2)));
      }
      currPos += 2;
    } else if (iter->value == "gpc") {  // gpc form
      currBlock.bytes.push_back(0x6F);
      requireNext(iter, tokens.cend());
      ++iter;
      expect(iter, "$");
      requireNext(iter, tokens.cend());
      ++iter;
      unsigned long buffer = getNumber(iter);
      if (buffer % 2 != 0)
        throw ParseError(iter->lineNo, iter->charNo,
                         iter->value + " must be divisible by two.");
      else if (buffer / 2 > 0xf)
        throw ParseError(
            iter->lineNo, iter->charNo,
            "out of range: half of " + iter->value + " must fit in 1 nibble.");

      requireNext(iter, tokens.cend());
      ++iter;
      expect(iter, ",");
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock.bytes.push_back(
          static_cast<uint8_t>(((buffer / 2) << 4) | getOneReg(iter)));
      currPos += 2;
    } else if (iter->value == "j") {  // j form
      requireNext(iter, tokens.cend());
      ++iter;
      if (iter->value == "*") {
        requireNext(iter, tokens.cend());
        ++iter;
        if (iter->value == "(") {
          requireNext(iter, tokens.cend());
          ++iter;
          uint8_t temp = getOneReg(iter);
          requireNext(iter, tokens.cend());
          ++iter;
          if (iter->value == ")") {
            // j * ( rd )
            currBlock.bytes.push_back(0xd0 | temp);
            currBlock.bytes.push_back(0x0);
          } else {
            // j * ( rd , ri , 4 )
            uint8_t base = temp;
            expect(iter, ",", ",' or '(");
            requireNext(iter, tokens.cend());
            ++iter;
            uint8_t index = getOneReg(iter);
            requireNext(iter, tokens.cend());
            ++iter;
            expect(iter, ",");
            requireNext(iter, tokens.cend());
            ++iter;
            expect(iter, "4");
            requireNext(iter, tokens.cend());
            ++iter;
            expect(iter, ")");
            currBlock.bytes.push_back(0xe0 | base);
            currBlock.bytes.push_back(index << 4);
          }
        } else {
          // j * o ( rd )
          unsigned long buffer = getNumber(iter);
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, "(");
          uint8_t rd = getOneReg(iter);
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, ")");
          currBlock.bytes.push_back(0xd0 | rd);
          if (buffer % 4 != 0)
            throw ParseError(iter->lineNo, iter->charNo,
                             iter->value + " must be divisible by four.");
          else if (buffer / 4 > 0xff)
            throw ParseError(iter->lineNo, iter->charNo,
                             "out of range: a quarter of " + iter->value +
                                 " must fit in 1 byte.");
          currBlock.bytes.push_back(static_cast<uint8_t>(buffer));
        }
      } else if (iter->value == "(") {
        // j ( rd )
        requireNext(iter, tokens.cend());
        ++iter;
        uint8_t rd = getOneReg(iter);
        requireNext(iter, tokens.cend());
        ++iter;
        expect(iter, ")");
        currBlock.bytes.push_back(0xc0 | rd);
        currBlock.bytes.push_back(0x0);
      } else {
        if (iter + 1 != tokens.cend() &&
            (iter + 1)->value == "(") {  // j o ( rd )
          requireNext(iter, tokens.cend());
          ++iter;
          unsigned long buffer = getNumber(iter);
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, "(");
          uint8_t rd = getOneReg(iter);
          requireNext(iter, tokens.cend());
          ++iter;
          expect(iter, ")");
          currBlock.bytes.push_back(0xc0 | rd);
          if (buffer % 2 != 0)
            throw ParseError(iter->lineNo, iter->charNo,
                             iter->value + " must be divisible by two.");
          else if (buffer / 2 > 0xff)
            throw ParseError(iter->lineNo, iter->charNo,
                             "out of range: half of " + iter->value +
                                 " must fit in 1 byte.");
          currBlock.bytes.push_back(static_cast<uint8_t>(buffer));
        } else {
          // requireNext(iter, tokens.cend());
          // ++iter;
          currBlock.bytes.push_back(0xb0);
          currBlock.bytes.push_back(0x0);
          uint32_t address;
          if (validLabel(iter->value)) {
            address = 0x5a5a5a5a;  // magic number - 0x5--- is an invalid opcode
            labelUses.push_back(LabelUse(currPos + 2, iter->value, iter->lineNo,
                                         iter->charNo, false));
          } else {
            address = getInt(iter);
          }
          addInt(address, currBlock);
          currPos += 4;
          // j const
        }
      }
      currPos += 2;
    } else if (iter->value == ".pos") {  //.pos form
      blocks.push_back(currBlock);
      requireNext(iter, tokens.cend());
      ++iter;
      currBlock = Block();
      currBlock.startPos = getInt(iter);
      currPos = currBlock.startPos;
    } else if (iter->value == ".long" ||
               iter->value == ".data") {  // literal data
      requireNext(iter, tokens.cend());
      ++iter;
      if (validLabel(iter->value))
        labelUses.push_back(
            LabelUse(currPos, iter->value, iter->lineNo, iter->charNo, false));
      else
        addInt(getInt(iter), currBlock);
      currPos += 4;
    } else if (validLabel(iter->value,
                          true)) {  // label binding
      // add label to labelBinds
      string labelName = iter->value.substr(0, iter->value.length() - 1);
      if (labelBinds.find(labelName) == labelBinds.end())
        labelBinds.insert(pair<string, uint32_t>(labelName, currPos));
      else
        throw ParseError(iter->lineNo, iter->charNo,
                         "cannot reuse label '" + labelName + "'.");
      continue;  // labels don't have to have a newline after them.
    } else if (iter->value == "\n") {
      continue;  // ignore extraneous newlines.
    } else {
      badToken(iter);
    }

    if (iter + 1 != tokens.cend()) {  // require newline
      ++iter;
      if (iter->value != "\n") {
        throw ParseError(iter->lineNo, iter->charNo,
                         "expected newline, but got '" + iter->value + "'.");
      }
    }
  }

  blocks.push_back(currBlock);

  vector<uint8_t> result = bytesFromBlocks(blocks);  // final processing steps
  replacePlaceholders(result, labelBinds, labelUses);

  return result;
}
}  // namespace sm213assemble::model