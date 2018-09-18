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

#include "io/tokenizer.h"

#include <algorithm>

namespace sm213assembler::io {
namespace {
using std::find;
using std::to_string;

const char* SPECIAL_SYMBOLS = "()$,:.";
}  // namespace

Token::Token(string v, int l, int c) noexcept
    : value{v}, lineNo{l}, charNo{c} {}

IllegalCharacter::IllegalCharacter(char character, int line,
                                   int column) noexcept
    : msg{to_string(line) + ":" + to_string(column) + ": Illegal character `" +
          to_string(character) + "`."} {}
const char* IllegalCharacter::what() const noexcept { return msg.c_str(); }

vector<Token> tokenize(ifstream& fin) {
  vector<Token> rsf;
  Token tokenBuffer("", 1, 1);
  char readBuffer;

  int currLine = 1, currChar = 1;
  bool inComment = false;

  while (fin.get(readBuffer)) {
    if (readBuffer ==
        '\n') {    // reached end of line - don't care if comment or no.
      currLine++;  // reset line count, commented flag
      currChar = 1;
      inComment = false;
      if (!tokenBuffer.value.empty())
        rsf.push_back(tokenBuffer);                 // record token if not empty
      tokenBuffer = Token("", currLine, currChar);  // reset token
    } else if (inComment) {  // in a comment - don't do anything with these
                             // chars.
      currChar++;
    } else if (readBuffer == '#') {  // start of comment
      inComment = true;              // turn start of comment to true
      currChar++;
    } else if (isblank(readBuffer)) {  // any whitespace except newline
      if (!tokenBuffer.value
               .empty())  // if isn't empty, record it and reset buffer
        rsf.push_back(tokenBuffer);
      currChar++;
      tokenBuffer =
          Token("", currLine, currChar);  // token doesn't start at previous
                                          // place, it starts here now.
    } else if (find(SPECIAL_SYMBOLS, SPECIAL_SYMBOLS + 6, readBuffer) !=
               SPECIAL_SYMBOLS + 6) {  // is a special symbol
      if (!tokenBuffer.value.empty())
        rsf.push_back(tokenBuffer);  // save token if not empty
      rsf.push_back(Token(string(readBuffer, 1), currLine,
                          currChar));  // save single char as token
      currChar++;
      tokenBuffer = Token("", currLine, currChar);
    } else if (isalnum(readBuffer)) {   // plain character
      tokenBuffer.value += readBuffer;  // save and move on
      currChar++;
    } else {
      throw IllegalCharacter(readBuffer, currLine, currChar);
    }
  }
}
}  // namespace sm213assembler::io