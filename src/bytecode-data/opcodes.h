#pragma once

#include <src/runtime/module-desc.h>
#include <src/lexer.h>

#include <stdint.h>
#include <stddef.h>

enum class opcode_t : uint16_t {
    clear_stack,

    jump_exe,  // unconditional jump
    jump_true,  // jump if TOS is true
    jump_false, // jump if TOS is false
    pop_scope, // reduce scope by one, this catches all for loops and if statements
    push_scope_for,
    push_scope_if,
    return_,   // returns from module, returning from module before outputs are assigned is bad

    push_true,
    push_false,

    push_in_ref,
    push_out_ref,
    push_new_local_ref,
    push_local_ref,
    push_uinteger,
    push_bit_literal,
    assign_in_ref,
    assign_out_ref,

    push_fn_args_sentinal,
    push_vec_args_sentinal,
    push_arr_sentinal,
    push_module_args_sentinal,
    function_call,
    module_call,
    index_call,
    set_interface_size,

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

    operator_unary_negate,
    operator_binary_not,
    operator_binary_xor,
    operator_binary_and,
    operator_binary_or,

    operator_range_desc, // begin:end

    push_new_local_any, // catch-all for everything not explicitly specified here
    push_new_local_integer,
    push_new_local_uinteger,
    push_new_local_string,
    push_new_local_vector,
    push_new_local_module,

};

namespace opc {

    void UNIMPLEMENTED(struct module_desc_t*);

    void clear_stack(struct module_desc_t* modptr);
    void jump_exe(struct module_desc_t* modptr, const size_t jump_label);
    void jump_on_false(struct module_desc_t* modptr, const size_t jump_label);
    void jump_on_true(struct module_desc_t* modptr, const size_t jump_label);

    void push_true(struct module_desc_t* modptr);
    void push_false(struct module_desc_t* modptr);

    void push_scope_if(struct module_desc_t* modptr);
    void push_scope_for(struct module_desc_t* modptr);

    void pop_scope(struct module_desc_t* modptr);
    void return_(struct module_desc_t* modptr);

    void push_in_ref(struct module_desc_t* modptr, const size_t ref);
    void push_out_ref(struct module_desc_t* modptr, const size_t ref);
    void push_local(struct module_desc_t* modptr, const size_t ref);
    void push_uinteger(struct module_desc_t* modptr, size_t u64);
    void push_bit_literal(struct module_desc_t* modptr, size_t ref);

    void push_new_local(struct module_desc_t* modptr, token_type_t t, const size_t ref);
    void push_new_local_any(struct module_desc_t* modptr, const size_t ref);
    void push_new_local_ref(struct module_desc_t* modptr, const size_t ref);
    void push_new_local_integer(struct module_desc_t* modptr, const size_t ref);
    void push_new_local_uinteger(struct module_desc_t* modptr, const size_t ref);
    void push_new_local_string(struct module_desc_t* modptr, const size_t ref);
    void push_new_local_vector(struct module_desc_t* modptr, const size_t ref);
    void push_new_local_module(struct module_desc_t* modptr, const size_t ref);

    void push_fn_args_sentinal(struct module_desc_t* modptr);
    void push_vec_args_sentinal(struct module_desc_t* modptr);
    void push_arr_sentinal(struct module_desc_t* modptr);
    void push_module_args_sentinal(struct module_desc_t* modptr);
    void function_call(struct module_desc_t* modptr, function_type_t fn_name);
    void module_call(struct module_desc_t* modptr, size_t module_ref);
    void set_interface_size(struct module_desc_t* modptr);

    namespace operator_ {

        void add(struct module_desc_t* modptr);
        void subtract(struct module_desc_t* modptr);
        void multiply(struct module_desc_t* modptr);
        void divide(struct module_desc_t* modptr);
        void assign(struct module_desc_t* modptr);
        void get_field(struct module_desc_t* modptr);
        void cmp_lt(struct module_desc_t* modptr);
        void cmp_le(struct module_desc_t* modptr);
        void cmp_gt(struct module_desc_t* modptr);
        void cmp_ge(struct module_desc_t* modptr);

        void unary_negate(struct module_desc_t* modptr);
        void binary_not(struct module_desc_t* modptr);

        void binary_xor(struct module_desc_t* modptr);
        void binary_and(struct module_desc_t* modptr);
        void binary_or(struct module_desc_t* modptr);

        void range_desc(struct module_desc_t* modptr);
        void index_call(struct module_desc_t* modptr);

    } // namespace operator

} // namespace opc




