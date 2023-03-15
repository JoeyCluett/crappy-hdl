#include <src/bytecode-data/opcodes.h>
#include <src/error-util.h>

#include <stdexcept>

static void opc_inst(struct module_desc_t* modptr, opcode_t opc) {
    uint16_t u16 = static_cast<uint16_t>(opc);
    if(u16 < 0b01111111) {
        modptr->bytecode.push_back((uint8_t)(u16 & 0xFF));
    } else if(u16 < 0b0011111111111111) {
        modptr->bytecode.push_back((uint8_t)(((u16 >> 7) & 0xFF) | 0x80));
        modptr->bytecode.push_back((uint8_t)(u16 & 0xFF));
    } else {
        modptr->bytecode.push_back((uint8_t)(((u16 >> 14) & 0xFF) | 0x80));
        modptr->bytecode.push_back((uint8_t)(((u16 >>  7) & 0xFF) | 0x80));
        modptr->bytecode.push_back((uint8_t)(u16 & 0xFF));
    }
}

static void opc_size_const(struct module_desc_t* modptr, const size_t u64) {
    // stores large ints in big-endian order, 7 bits at a time
    static_assert(sizeof(size_t) == 8);

    size_t chunks = 0;
    for(size_t i = 0ul; i < 10ul; i++) {
        const size_t shftamt = 7ul * i;
        const size_t tmp = u64 >> shftamt;
        if((tmp & 0b01111111) == tmp) {
            chunks = i;
            break;            
        }
    }

    for(; chunks > 0; chunks--) {
        const size_t shftamt = 7ul * chunks;
        const size_t tmp = ((u64 >> shftamt) & 0b01111111) | 0b10000000; // intermediate chunks always prepended with 1
        modptr->bytecode.push_back((uint8_t)(tmp & 0xFF));
    }

    modptr->bytecode.push_back((uint8_t)(u64 & 0x7F)); // last chunk always prepended with 0 to indicate end
}

void opc::UNIMPLEMENTED(struct module_desc_t*) {
    throw std::runtime_error("unimplemented");
}

void opc::clear_stack(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::clear_stack);
}

void opc::jump_exe(struct module_desc_t* modptr, const size_t jump_label) {
    opc_inst(modptr, opcode_t::jump_exe);
    opc_size_const(modptr, jump_label);
}

void opc::jump_on_false(struct module_desc_t* modptr, const size_t jump_label) {
    opc_inst(modptr, opcode_t::jump_false);
    opc_size_const(modptr, jump_label);
}

void opc::jump_on_true(struct module_desc_t* modptr, const size_t jump_label) {
    opc_inst(modptr, opcode_t::jump_true);
    opc_size_const(modptr, jump_label);
}

void opc::push_true(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::push_true);
}

void opc::push_false(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::push_false);
}

void opc::set_interface_size(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::set_interface_size);
}

void opc::push_fn_args_sentinal(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::push_fn_args_sentinal);
}

void opc::push_vec_args_sentinal(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::push_vec_args_sentinal);
}

void opc::push_arr_sentinal(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::push_arr_sentinal);
}

void opc::push_module_args_sentinal(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::push_module_args_sentinal);
}

void opc::function_call(struct module_desc_t* modptr, function_type_t fn_name) {
    opc_inst(modptr, opcode_t::function_call);
    modptr->bytecode.push_back(static_cast<uint8_t>(fn_name));
}

void opc::module_call(struct module_desc_t* modptr, size_t module_ref) {
    opc_inst(modptr, opcode_t::module_call);
    opc_size_const(modptr, module_ref);
}


void opc::push_in_ref(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_in_ref);
    opc_size_const(modptr, ref);
}

void opc::push_out_ref(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_out_ref);
    opc_size_const(modptr, ref);
}

void opc::push_new_local_ref(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_new_local_ref);
    opc_size_const(modptr, ref);
}

void opc::push_local(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_local_ref);
    opc_size_const(modptr, ref);
}

void opc::push_uinteger(struct module_desc_t* modptr, size_t u64) {
    opc_inst(modptr, opcode_t::push_uinteger);
    opc_size_const(modptr, u64);
}

void opc::push_bit_literal(struct module_desc_t* modptr, size_t ref) {
    opc_inst(modptr, opcode_t::push_bit_literal);
    opc_size_const(modptr, ref);
}

void opc::push_new_local(struct module_desc_t* modptr, token_type_t t, const size_t ref) {
    switch(t) {
    case token_type_t::keyword_integer:  return opc::push_new_local_integer(modptr, ref);
    case token_type_t::keyword_uinteger: return opc::push_new_local_uinteger(modptr, ref);
    case token_type_t::keyword_string:   return opc::push_new_local_string(modptr, ref);
    case token_type_t::keyword_vector:   return opc::push_new_local_vector(modptr, ref);
    case token_type_t::keyword_module:   return opc::push_new_local_module(modptr, ref);
    default:
        INTERNAL_ERR();
    }
}

void opc::push_new_local_any(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_new_local_any);
    opc_size_const(modptr, ref);
}

void opc::push_new_local_integer(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_new_local_integer);
    opc_size_const(modptr, ref);
}

void opc::push_new_local_uinteger(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_new_local_uinteger);
    opc_size_const(modptr, ref);
}

void opc::push_new_local_string(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_new_local_string);
    opc_size_const(modptr, ref);
}

void opc::push_new_local_vector(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_new_local_vector);
    opc_size_const(modptr, ref);
}

void opc::push_new_local_module(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_new_local_module);
    opc_size_const(modptr, ref);
}

void opc::pop_scope(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::pop_scope);
}

void opc::push_scope_if(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::push_scope_if);
}

void opc::push_scope_for(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::push_scope_for);
}

void opc::return_(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::return_);
}

namespace opc {
namespace operator_ {

    void add(struct module_desc_t* modptr)       { opc_inst(modptr, opcode_t::operator_add);       }
    void subtract(struct module_desc_t* modptr)  { opc_inst(modptr, opcode_t::operator_subtract);  }
    void multiply(struct module_desc_t* modptr)  { opc_inst(modptr, opcode_t::operator_multiply);  }
    void divide(struct module_desc_t* modptr)    { opc_inst(modptr, opcode_t::operator_divide);    }
    void assign(struct module_desc_t* modptr)    { opc_inst(modptr, opcode_t::operator_assign);    }
    void get_field(struct module_desc_t* modptr) { opc_inst(modptr, opcode_t::operator_get_field); }

    void cmp_lt(struct module_desc_t* modptr)    { opc_inst(modptr, opcode_t::operator_cmp_lt);    }
    void cmp_le(struct module_desc_t* modptr)    { opc_inst(modptr, opcode_t::operator_cmp_le);    }
    void cmp_gt(struct module_desc_t* modptr)    { opc_inst(modptr, opcode_t::operator_cmp_gt);    }
    void cmp_ge(struct module_desc_t* modptr)    { opc_inst(modptr, opcode_t::operator_cmp_ge);    }

    void unary_negate(struct module_desc_t* modptr) { opc_inst(modptr, opcode_t::operator_unary_negate); }
    void binary_not(struct module_desc_t* modptr)   { opc_inst(modptr, opcode_t::operator_binary_not); }
    void binary_xor(struct module_desc_t* modptr) { opc_inst(modptr, opcode_t::operator_binary_xor); }
    void binary_and(struct module_desc_t* modptr) { opc_inst(modptr, opcode_t::operator_binary_and); }
    void binary_or(struct module_desc_t* modptr) { opc_inst(modptr, opcode_t::operator_binary_or); }

    void range_desc(struct module_desc_t* modptr) { opc_inst(modptr, opcode_t::operator_range_desc); }
    void index_call(struct module_desc_t* modptr) { opc_inst(modptr, opcode_t::index_call); }

} // namespace operator
} // namespace opc
