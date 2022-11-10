#pragma once

#include <stdint.h>
#include <stddef.h>

enum class opcode_t {
    push_symbol,
    push_integer,
    push_uinteger,
    push_short,
    push_ushort,

    assign, // =
    index,  // [] for vector and bitvec access
    access, // .

    push_builtin_module,
    push_module_ctor,
    call_module,
};

enum class builtin_module_t : uint8_t {
    match,
    decoder,
    cmpeq,
};

