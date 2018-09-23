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

#include "io.h"

#include <algorithm>

namespace sm213assembler::io {
namespace {
using std::find;
using std::to_string;

const char* SPECIAL_SYMBOLS = "()$,*";
const char* PSEUDO_ALPHA = "_.:";
}  // namespace

Token::Token(string v, unsigned l, unsigned c) noexcept
    : value{v}, lineNo{l}, charNo{c} {}

const char* FileOpenError::what() const noexcept { return ""; }

IllegalCharacter::IllegalCharacter(char character, unsigned line,
                                   unsigned column) noexcept
    : msg{to_string(line) + ":" + to_string(column) +
          ":illegal character: " + string(1, character)} {}
const char* IllegalCharacter::what() const noexcept { return msg.c_str(); }

vector<Token> tokenize(ifstream& fin) {
  vector<Token> rsf;
  Token tokenBuffer("", 1, 1);
  char readBuffer;

  unsigned currLine = 1, currChar = 1;
  bool inComment = false;

  while (fin.get(readBuffer)) {
    if (readBuffer ==
        '\n') {  // reached end of line - don't care if comment or no.
      Token temp = Token("\n", currLine, currChar);
      currLine++;  // reset line count, commented flag
      currChar = 1;
      inComment = false;
      if (!tokenBuffer.value.empty())
        rsf.push_back(tokenBuffer);  // record token if not empty
      rsf.push_back(temp);
      tokenBuffer = Token("", currLine, currChar);  // reset token
    } else if (inComment) {  // in a comment - don't do anything with these
                             // chars.
      currChar++;
    } else if (readBuffer == '#') {  // start of comment
      inComment = true;              // turn start of comment to true
      currChar++;
    } else if (find(SPECIAL_SYMBOLS, SPECIAL_SYMBOLS + 5, readBuffer) !=
               SPECIAL_SYMBOLS + 5) {  // is a special symbol
      if (!tokenBuffer.value.empty())
        rsf.push_back(tokenBuffer);  // save token if not empty
      rsf.push_back(Token(string(1, readBuffer), currLine,
                          currChar));  // save single char as token
      currChar++;
      tokenBuffer = Token("", currLine, currChar);
    } else if (isblank(readBuffer)) {  // any whitespace except newline
      if (!tokenBuffer.value
               .empty())  // if isn't empty, record it and reset buffer
        rsf.push_back(tokenBuffer);
      currChar++;
      tokenBuffer =
          Token("", currLine, currChar);  // token doesn't start at previous
                                          // place, it starts here now.
    } else if (isalnum(readBuffer) ||
               find(PSEUDO_ALPHA, PSEUDO_ALPHA + 3, readBuffer) !=
                   PSEUDO_ALPHA + 3) {  // plain character
      tokenBuffer.value += readBuffer;  // save and move on
      currChar++;
    } else if (readBuffer == '\r') {  // ignore carriage returns - shouldn't be
                                      // needed - io automatically purges these.
    } else {
      throw IllegalCharacter(readBuffer, currLine, currChar);
    }
  }

  return rsf;
}

void writeBinary(const vector<uint8_t>& binary, const string& fn) {
  ofstream fout;
  fout.open(fn,
            std::ios_base::binary | std::ios_base::trunc | std::ios_base::out);

  if (!fout.is_open()) throw FileOpenError();

  for (uint8_t byte : binary) fout << byte;

  fout.flush();
  fout.close();
  return;
}
}  // namespace sm213assembler::io