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
using std::exception;
using std::ofstream;
using std::string;
using std::vector;
}  // namespace

// AssemblyStatement ::= <LabelStatemet> <DotStatement>
//                     | <LabelStatemet> <OpcodeStatement>
// DotStatement ::= .pos <HexLiteral>
//                | .(long|data) <HexLiteral>
// HexLiteral ::= any hex literal
// Label ::= [a-zA-Z_][a-zA-Z_0-9]*
// LabelStatement ::= <Label> :
// Register ::= r[0-7]
// OpCodeStatement ::= ld $<Label> , <Register> // ld immediate using label
//                   | ld $<HexLiteral (uint)> , <Register> // ld literal
//                   | ld ( <Register> ) , <Register> // ld offset sugared
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
//                   | sh[lr] $ <HexLiteral [0, 0x7f]> , <Register>
//                   | gpc $ <HexLiteral> / by 2, [0, 0x1e], <Register>
//                   | j <HexLiteral, uint>
//                   | j ( <Register> )
//                   | j <HexLiteral, / by 2, [0, 0x1e]> ( <Register> )
//                   | j * <HexLiteral, / by 4, [0, 0x3c]> ( <Register> )
//                   | j * ( <Register> , <Register> , 4 )
//                   | br <HexLiteral, / by 2, 2's c [0x80, 0x7f]>
//                   | beq <Register> , <HexLiteral, / by 2, 2's c [0x80, 0x7f]>
//                   | bgt <Register> , <HexLiteral, / by 2, 2's c [0x80, 0x7f]>

// Unused code - delete if not needed before commit.

// namespace ast {
// namespace {
// using AS = AssemblyStatement;
// }

// struct Position : AS {
//   uint32_t newPos;
// };
// struct Literal : AS {
//   uint32_t literal;
// };
// struct LoadLiteral : AS {
//   uint32_t literal;
// };
// struct LoadLabel : AS {
//   string targetLabel;
// };
// struct LoadOffset : AS {
//   uint8_t destination;
//   uint8_t base;
//   uint8_t offset;
// };
// struct LoadIndexed : AS {
//   uint8_t destination;
//   uint8_t source;
//   uint8_t offset;
// };
// struct StoreOffset : AS {
//   uint8_t source;
//   uint8_t base;
//   uint8_t offset;
// };
// struct StoreIndexed : AS {
//   uint8_t source;
//   uint8_t destination;
//   uint8_t offset;
// };
// struct Halt : AS {};  // intentionally empty
// struct Nop : AS {};   // intentionally empty
// struct BinaryOperator : AS {
//   uint8_t opNum;  // one of 0, 1, 2 for move, add, and
//   uint8_t operand;
//   uint8_t target;
// };
// struct UnaryOperator : AS {
//   uint8_t opNum;  // one of 3, 4, 5, 6, 7 for inc, inca, dec, deca, not
//   uint8_t target;
// };
// struct ShiftOperator : AS {
//   int8_t shiftAmount;
//   uint8_t target;
// };
// struct GetPC : AS {
//   uint8_t offset;
//   uint8_t destination;
// };
// struct Jump : AS {
//   uint32_t literal;
// };
// struct JumpIndirect : AS {
//   uint8_t base;
//   uint8_t offset;
// };
// struct JumpDInd : AS {
//   uint8_t base;
//   uint8_t offset;
// };
// struct JumpDIndIndexed : AS {
//   uint8_t source;
//   uint8_t offset;
// };
// struct Branch : AS {
//   int8_t literal;
// };
// struct BranchCond : AS {
//   uint8_t comparison;  // one of 0x9, 0xa for equal, greater than
//   int8_t literal;
// };
// }  // namespace ast

class ParseError : public exception {
 public:
  ParseError(unsigned lineNo, unsigned charNo, string msg) noexcept;
  const char* what() const noexcept override;

 private:
  string msg;
};

vector<uint8_t> generateBinary(const vector<Token>&);
}  // namespace sm213assembler::model

#endif  // SM213ASSEMBLER_MODEL_AST_H_