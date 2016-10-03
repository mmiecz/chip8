//
// Created by mhl on 01.10.16.
//

#ifndef CHIP8_INSTRUCTION_SET_H
#define CHIP8_INSTRUCTION_SET_H

#include <tuple>
#include <functional>

class CPU;
//(opcode, mnemo, handler)
typedef std::tuple<std::uint16_t, std::string, std::function<void(CPU &)>> instruction;
#endif //CHIP8_INSTRUCTION_SET_H
