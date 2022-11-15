#include <src/bytecode-data/opcodes.h>

static void opc_push_in_ref(struct module_desc_t* modptr, const size_t ref);
static void opc_push_out_ref(struct module_desc_t* modptr, const size_t ref);
static void opc_push_local_ref(struct module_desc_t* modptr, const size_t ref);
static void opc_push_uinteger(struct module_desc_t* modptr, size_t u64);
static void opc_push_fn_args_sentinal(struct module_desc_t* modptr);

static void opc_set_interface_input_size(struct module_desc_t* modptr, const size_t ref);
static void opc_set_interface_output_size(struct module_desc_t* modptr, const size_t ref);

const opcode_interface_t opc = {
    .push_in_ref           = opc_push_in_ref,
    .push_out_ref          = opc_push_out_ref,
    .push_local_ref        = opc_push_local_ref,
    .push_uinteger         = opc_push_uinteger,

    .push_fn_args_sentinal = opc_push_fn_args_sentinal,

    .set_interface_input_size  = opc_set_interface_input_size,
    .set_interface_output_size = opc_set_interface_output_size,


};

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

static void opc_set_interface_input_size(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::set_interface_input_size);
    opc_size_const(modptr, ref);
}

static void opc_set_interface_output_size(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::set_interface_output_size);
    opc_size_const(modptr, ref);
}

static void opc_push_fn_args_sentinal(struct module_desc_t* modptr) {
    opc_inst(modptr, opcode_t::push_fn_args_sentinal);
}

static void opc_push_in_ref(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_in_ref);
    opc_size_const(modptr, ref);
}

static void opc_push_out_ref(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_out_ref);
    opc_size_const(modptr, ref);
}

static void opc_push_local_ref(struct module_desc_t* modptr, const size_t ref) {
    opc_inst(modptr, opcode_t::push_local_ref);
    opc_size_const(modptr, ref);
}

static void opc_push_uinteger(struct module_desc_t* modptr, size_t u64) {
    opc_inst(modptr, opcode_t::push_uinteger);
    opc_size_const(modptr, u64);
}
