#include <src/bytecode-data/opcodes.h>
#include <src/error-util.h>

static void opc_inst(struct module_desc_t* modptr, opcode_t opc) {
    union {
        uint16_t u16;
        uint8_t u8[2];
    };

    u16 = static_cast<uint16_t>(opc);
    modptr->bytecode.push_back(u8[0]);
    modptr->bytecode.push_back(u8[1]);
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

void opc::clear_stack(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::clear_stack);
}

void opc::set_interface_input_size(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::set_interface_input_size);
}

void opc::set_interface_output_size(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::set_interface_output_size);
}

void opc::push_fn_args_sentinal(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::push_fn_args_sentinal);
}

void opc::function_call(struct module_desc_t* modptr, function_type_t fn_name) {
    opc_inst(modptr, opcode_t::function_call);
    modptr->bytecode.push_back(static_cast<uint8_t>(fn_name));
}

void opc::push_in_ref(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_in_ref);
    opc_size_const(modptr, ref);
}

void opc::push_out_ref(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_out_ref);
    opc_size_const(modptr, ref);
}

void opc::push_local_ref(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_local_ref);
    opc_size_const(modptr, ref);
}

void opc::push_uinteger(struct module_desc_t* modptr, size_t u64) {
    opc_inst(modptr, opcode_t::push_uinteger);
    opc_size_const(modptr, u64);
}

void opc::push_new_local(struct module_desc_t* modptr, token_type_t t, const size_t ref) {
    switch(t) {
    case token_type_t::keyword_integer:  return opc::push_new_local_integer(modptr, ref);
    case token_type_t::keyword_uinteger: return opc::push_new_local_uinteger(modptr, ref);
    case token_type_t::keyword_string:   return opc::push_new_local_string(modptr, ref);
    case token_type_t::keyword_vector:   return opc::push_new_local_vector(modptr, ref);
    default:
        INTERNAL_ERR();
    }
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
