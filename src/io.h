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

#ifndef SM213ASSEMBLER_IO_H_
#define SM213ASSEMBLER_IO_H_

#include <fstream>
#include <stdexcept>
#include <vector>

namespace sm213assembler::io {
namespace {
using std::exception;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;
}  // namespace

struct Token {
  string value;
  unsigned lineNo;
  unsigned charNo;

  Token(string value, unsigned lineNo, unsigned charNo) noexcept;
};

class FileOpenError : public exception {
 public:
  FileOpenError() noexcept = default;
  const char* what() const noexcept override;
};
class IllegalCharacter : public exception {
 public:
  IllegalCharacter(char character, unsigned line, unsigned column) noexcept;
  const char* what() const noexcept override;

 private:
  string msg;
};

void writeBinary(const vector<uint8_t>&, const string&);
vector<Token> tokenize(ifstream&);
}  // namespace sm213assembler::io

#endif  // SM213ASSEMBLER_IO_H_