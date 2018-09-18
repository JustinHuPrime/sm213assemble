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

namespace sm213assembler::model {
namespace {
using namespace ast;  // bad style, but I really want to import all of ast.
using std::all_of;
using std::find;
using std::to_string;

bool validLabel(string s) {
  return all_of(s.begin(), s.end() - 1,
                [](char c) { return isalnum(c) || c == '_'; }) &&
         !isdigit(s.front()) && s.back() == ':';
}
}  // namespace

ParseError::ParseError(unsigned l, unsigned c, string m) noexcept
    : msg{to_string(l) + ":" + to_string(c) + ":" + m} {}
const char* ParseError::what() const noexcept { return msg.c_str(); }

vector<AssemblyStatement> makeAst(const vector<Token>& tokens) {
  vector<AssemblyStatement> rsf;

  string labelAcc;
  for (auto iter = tokens.cbegin(); iter != tokens.cend(); ++iter) {
    // iter is one of: special symbol, string, hex constant
    // at this point, just started reading, have read a label, or have
    // already read a statement.

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
               iter->value == ".data") {   // literal data
    } else if (validLabel(iter->value)) {  // label
      labelAcc = iter->value.substr(
          0, iter->value.length() - 1);  // set the label (remove colon)
      ++iter;  // parse the thing attached to this label
      continue;
    } else {
      throw ParseError(iter->lineNo, iter->charNo,
                       "unrecognized token '" + iter->value + "'.");
    }

    labelAcc = "";  // parsed one whole thing - can't be any more label
  }
}
void generateBinary(vector<AssemblyStatement>&, ofstream& fout) noexcept {
  return;
}
}  // namespace sm213assembler::model