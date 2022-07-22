#pragma once

#include <stdint.h>

enum Opcode_t : uint8_t {

    //
    // [1x Opcode] [1x datatype] [1x name table index]
    //
    // datatype:
    //      0x00 : long
    //      0x01 : ulong
    //      0x02 : string
    //      0x03 : any (used for references to io)
    //      0x04 : vector
    //
    Opcode_DECLARE_NEW_LOCAL,     // [1x Opcode] [1x datatype] [1x name table index], 3 bytes
    Opcode_DECLARE_NEW_LOCAL_EXT, // [1x Opcode] [1x datatype] [1x name table index (starts at 256)], 3 bytes

    Opcode_PUSH_LOCAL,            // [1x Opcode] [1x name table index (starts at 0)], 2 bytes
    Opcode_PUSH_LOCAL_EXT,        // [1x Opcode] [1x name table index (starts at 256)], 2 bytes

    //
    // [1x Opcode] [1x builtin index]
    //
    // builtin index:
    //      0x00 : cmpeq
    //      0x01 : cmpneq
    //      0x02 : decoder
    //      0x03 : match
    //
    Opcode_MODULE_BUILTIN,     // [1x Opcode] [1x builtin index], 2 bytes
    Opcode_MODULE_BUILTIN_EXT, // [1x Opcode] [1x builtin index (extended set)], 2 bytes. currently no extended builtin types

    Opcode_RANGE_SENTINAL = 27,

    Opcode_PUSH_STRING_LITERAL = 28,
    Opcode_PUSH_INT_LITERAL    = 29,
    Opcode_PUSH_UINT_LITERAL   = 30,

};

