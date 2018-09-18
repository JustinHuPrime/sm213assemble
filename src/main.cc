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

// SM213 assembler takes one command line argument - target .sm213 file, then
// assembles target file into a .img file. Resulting file name is source file
// name with changed extension.

#include "io/tokenizer.h"
#include "io/writer.h"
#include "model/ast.h"

#include <fstream>
#include <iostream>
#include <vector>

namespace {
using sm213assembler::io::FileOpenError;
using sm213assembler::io::IllegalCharacter;
using sm213assembler::io::Token;
using sm213assembler::io::tokenize;
using sm213assembler::io::writeBinary;
using sm213assembler::model::generateBinary;
using sm213assembler::model::ParseError;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;
}  // namespace

int main(int argc, char* argv[]) {
  if (argc != 2) {
    cerr << "Expected source file as only argument.\n";
    return EXIT_FAILURE;
  }

  string sourceFileName(argv[1]);
  string destinationFileName = sourceFileName;
  destinationFileName.replace(sourceFileName.find_last_of('.'),
                              sourceFileName.length(), ".img");

  ifstream fin;
  fin.open(sourceFileName);

  if (!fin.is_open()) {
    cerr << sourceFileName << '\n';
    cerr << "Could not open source file. Aborting.\n";
    return EXIT_FAILURE;
  }

  vector<Token> tokens;
  try {
    tokens = tokenize(fin);
  } catch (const IllegalCharacter& e) {
    cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  vector<uint8_t> binary;
  try {
    binary = generateBinary(tokens);
  } catch (const ParseError& e) {
    cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  try {
    writeBinary(binary, destinationFileName);
  } catch (const FileOpenError&) {
    cerr << "Could not open output file. Aborting.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}