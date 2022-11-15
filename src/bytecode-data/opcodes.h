#pragma once

#include <src/runtime/module-desc.h>

#include <stdint.h>
#include <stddef.h>

enum class opcode_t : uint16_t {
    push_in_ref,
    push_out_ref,
    push_local_ref,
    push_uinteger,
    assign_in_ref,
    assign_out_ref,

    push_fn_args_sentinal,
    set_interface_input_size,
    set_interface_output_size,

    operator_add,
    operator_subtract,
    operator_multiply,
    operator_divide,
    operator_assign,

    operator_get_field,

    operator_cmp_lt,
    operator_cmp_le,
    operator_cmp_gt,
    operator_cmp_ge,

};

enum class function_ref_t : uint8_t {
    last, // returns reference to last element in array/vector type
    push, // for vector types
};

struct opcode_interface_t {
    void(*push_in_ref)(struct module_desc_t* modptr, const size_t ref);
    void(*push_out_ref)(struct module_desc_t* modptr, const size_t ref);
    void(*push_local_ref)(struct module_desc_t* modptr, const size_t ref);
    void(*push_uinteger)(struct module_desc_t* modptr, size_t u64);

    void(*push_fn_args_sentinal)(struct module_desc_t* modptr);

    void(*set_interface_input_size)(struct module_desc_t* modptr, const size_t ref);
    void(*set_interface_output_size)(struct module_desc_t* modptr, const size_t ref);
};

extern const opcode_interface_t opc;



