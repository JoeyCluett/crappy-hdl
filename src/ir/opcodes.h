#pragma once

#include <stdint.h>

enum Opcode_t : uint8_t {

    //
    // [1x 0x00] [1x datatype] [1x name table index]
    //
    // datatype:
    //      0x00 : long
    //      0x01 : ulong
    //      0x02 : string
    //
    Opcode_DECLARE_NEW_LOCAL =  0, //     [1x 0x00] [1x datatype] [1x name table index], 3 bytes
    Opcode_PUSH_LOCAL        =  1, //     [1x 0x01] [1x name table index], 2 bytes

    Opcode_ASSIGN            =  2, //     [1x 0x02], 1 byte
    Opcode_ADD               =  3, //     [1x 0x03], 1 byte
    Opcode_SUBTRACT          =  4, // -   [1x 0x04], 1 byte
    Opcode_MULTIPLY          =  5, // *   [1x 0x05], 1 byte
    Opcode_DIVIDE            =  6, // /   [1x 0x06], 1 byte
    Opcode_EVAL_INDEX        =  7, // []  [1x 0x07], 1 byte
    Opcode_EVAL_RANGE        =  8, // {}  [1x 0x08], 1 byte
    Opcode_BIT_OR            =  9, // |   [1x 0x09], 1 byte
    Opcode_BIT_AND           = 10, // &   [1x 0x0A], 1 byte
    Opcode_BIT_XOR           = 11, // ^   [1x 0x0B], 1 byte
    Opcode_NOT               = 12, // !   [1x 0x0C], 1 byte
    Opcode_INVERT            = 13, // ~   [1x 0x0D], 1 byte
    Opcode_NEGATE            = 14, // -   [1x 0x0E], 1 byte
    Opcode_EQUIVALENT        = 15, // ==  [1x 0x0F], 1 byte
    Opcode_NOT_EQUIVALENT    = 16, // !=  [1x 0x10], 1 byte

    //
    // [1x 0x11] [1x builtin index]
    //
    // builtin index:
    //      0x00 : cmpeq
    //      0x01 : cmpneq
    //      0x02 : decoder
    //
    Opcode_MODULE_BUILTIN    = 17, // [1x 0x11] [1x builtin index], 2 bytes

    Opcode_FUNCTION_SENTINAL = 18, // [1x 0x12], 1 byte, marks beginning of function arguments
    Opcode_MODULE_CALL       = 19, // [1x 0x13], 1 byte
    Opcode_FN_PRINT          = 20, // [1x 0x14], 1 byte
    Opcode_FN_PRINTLN        = 21, // [1x 0x15], 1 byte
    Opcode_FN_MIN            = 22, // [1x 0x16], 1 byte
    Opcode_FN_MAX            = 23, // [1x 0x17], 1 byte

    // extended forms of previous instructions to address more locals
    Opcode_DECLARE_NEW_LOCAL_EXT = 24, // [1x 0x18] [1x datatype] [2x name table index], 4 bytes
    Opcode_PUSH_LOCAL_EXT        = 25, // [1x 0x19] [2x name table index], 3 bytes
    Opcode_MODULE_BUILTIN_EXT    = 26, // [1x 0x1A] [2x builtin index], 3 bytes

    Opcode_RANGE_SENTINAL = 27,

};

