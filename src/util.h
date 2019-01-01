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

#ifndef SM213ASSEMBLE_UTIL_H_
#define SM213ASSEMBLE_UTIL_H_

#include <sstream>
#include <string>

namespace sm213assemble::util {
namespace {
using std::string;
using std::stringstream;
}  // namespace
string hexify(long value);
}  // namespace sm213assemble::util

#endif  // SM213ASSEMBLE_UTIL_H_