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

#ifndef SM213ASSEMBLER_MODEL_AST_H_
#define SM213ASSEMBLER_MODEL_AST_H_

#include "io/tokenizer.h"

#include <fstream>
#include <vector>

namespace sm213assembler::model {
namespace {
using sm213assembler::io::Token;
using std::ofstream;
using std::vector;
}  // namespace

// AssemblyStatement ::= <DotStatement>
//                     | <Label> <OpcodeStatement>
// DotStatement ::= . pos <HexLiteral>
//                | <LabelStatement> . long <HexLiteral>
// HexLiteral ::= 0x[0-9]+
// Label ::= [a-zA-Z_][a-zA-Z_0-9]*
// LabelStatement ::= <Label> :
// Register ::= r[0-7]
// OpCodeStatement ::= ld $<Label> , <Register> // ld immediate using label
//                   | ld $<HexLiteral (int)> , <Register> // ld literal
//                   | ld ( <Register> ) , <Register> // ld indexed sugared
//                   | ld <HexLiteral / by 4, [0x0, 0x3c]> ( <Register> ) ,
//                   <Register> // ld offset
//                                                                    no sugar
//                   | ld ( <Register> , <Register> , 4 ) <Register> // ld index
//                   | st <Register> , ( <Register> ) // st offset sugared
//                   | st <Register> , <HexLiteral / by 4, [0, 60]> ( <Register>
//                   ) // st offset
//                                                                    no sugar
//                   | st <Register> , ( <Register> , <Register> , 4 ) // st
//                                                                        index
//                   | halt
//                   | nop
//                   | <BinaryOperator> <Register> , <Register>
//                   | <UnaryOperator> <Register>
//                   | shl $ <HexLiteral 2's c [0x80, 0x7f]> , <Register>
//                   | gpc $ <HexLiteral> / by 2, [0, 0x1e], <Register>
//                   | j <HexLiteral, uint>
//                   | j ( <Register> )
//                   | j <HexLiteral, / by 2, [0, 0x1e]> ( <Register> )
//                   | j * <HexLiteral, / by 4, [0, 0x3c]> ( <Register> )
//                   | j * ( <Register> , <Register> , 4 )
//                   | br <HexLiteral, / by 2, 2's c [0x80, 0x7f]>
//                   | beq <Register> , <HexLiteral, / by 2, 2's c [0x80, 0x7f]>
//                   | bgt <Register> , <HexLiteral, / by 2, 2's c [0x80, 0x7f]>

struct AssemblyStatement {
  uint sourceLineNo;
  uint sourceCharNo;
};

struct PositionStatement : AssemblyStatement {
  uint32_t newPos;
};

vector<AssemblyStatement> makeAst(vector<Token>&);
void generateBinary(vector<AssemblyStatement>&, ofstream&) noexcept;
}  // namespace sm213assembler::model

#endif  // SM213ASSEMBLER_MODEL_AST_H_