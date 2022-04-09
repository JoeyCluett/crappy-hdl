#pragma once

#include <stdint.h>

//
// [1x 0x00] [1x datatype] [4x name table index]
//
// type:
//      0x00 : long
//      0x01 : ulong
//      0x02 : string
//
const uint8_t Opcode_DECLARE_NEW_LOCAL = 0;

//
// [1x 0x01] [4x name table index]
//
const uint8_t Opcode_PUSH_LOCAL = 1;

//
// [1x 0x02] -- behavior is highly runtime-dependent
//
const uint8_t Opcode_ASSIGN = 2;


