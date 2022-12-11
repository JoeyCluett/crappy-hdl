#pragma once

#include <src/runtime/module-desc.h>
#include <src/lexer.h>

#include <stdint.h>
#include <stddef.h>

enum class opcode_t : uint16_t {
    clear_stack,

    push_in_ref,
    push_out_ref,
    push_local_ref,
    push_uinteger,
    assign_in_ref,
    assign_out_ref,

    push_fn_args_sentinal,
    function_call,

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

    push_new_local_integer,
    push_new_local_uinteger,
    push_new_local_string,
    push_new_local_vector,

};

namespace opc {

    void clear_stack(struct module_desc_t* modptr);

    void push_in_ref(struct module_desc_t* modptr, const size_t ref);
    void push_out_ref(struct module_desc_t* modptr, const size_t ref);
    void push_local_ref(struct module_desc_t* modptr, const size_t ref);
    void push_uinteger(struct module_desc_t* modptr, size_t u64);
    void set_interface_input_size(struct module_desc_t* modptr);
    void set_interface_output_size(struct module_desc_t* modptr);

    void push_new_local(struct module_desc_t* modptr, token_type_t t, const size_t ref);
    void push_new_local_integer(struct module_desc_t* modptr, const size_t ref);
    void push_new_local_uinteger(struct module_desc_t* modptr, const size_t ref);
    void push_new_local_string(struct module_desc_t* modptr, const size_t ref);
    void push_new_local_vector(struct module_desc_t* modptr, const size_t ref);

    void push_fn_args_sentinal(struct module_desc_t* modptr);
    void function_call(struct module_desc_t* modptr, function_type_t fn_name);

} // namespace opc




