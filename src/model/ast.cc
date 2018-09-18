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

namespace sm213assembler::model {
namespace {}  // namespace

vector<AssemblyStatement> makeAst(vector<Token>&) {
  return vector<AssemblyStatement>();
}
void generateBinary(vector<AssemblyStatement>&, ofstream& fout) { return; }
}  // namespace sm213assembler::model