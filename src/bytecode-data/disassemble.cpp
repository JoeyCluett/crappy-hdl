#include <src/bytecode-data/disassemble.h>
#include <src/bytecode-data/opcodes.h>
#include <src/runtime/module-desc.h>
#include <src/error-util.h>

static size_t dis_get_ref(
        std::vector<uint8_t>::iterator& iter,
        std::vector<uint8_t>::iterator& end);

static std::string dis_get_ref_name(
        struct module_desc_t* modptr,
        size_t ref);

static opcode_t dis_get_opcode(
        std::vector<uint8_t>::iterator& iter);

void disassemble_bytecode(std::ostream& os, struct module_desc_t* modptr) {

    os << "\n\n";

    auto opc_iter = modptr->bytecode.begin();
    auto opc_end  = modptr->bytecode.end();

    while(opc_iter < opc_end) {

        switch(dis_get_opcode(opc_iter)) {
        case opcode_t::clear_stack: os << "clear_stack\n"; break;
        case opcode_t::push_in_ref: { // <opc> <ref>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_in_ref [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        case opcode_t::push_out_ref: { // <opc> <ref>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_out_ref [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        case opcode_t::push_local_ref: { // <opc> <ref>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_local_ref [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        case opcode_t::push_uinteger: { // <opc> <uint>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_uinteger " << ref << "\n";
            break;
        }

        case opcode_t::assign_in_ref: // <opc>
            os << "assign_in_ref\n";
            break;

        case opcode_t::assign_out_ref: // <opc>
            os << "assign_out_ref\n";
            break;

        case opcode_t::push_fn_args_sentinal: // <opc>
            os << "push_fn_args_sentinal\n";
            break;

        case opcode_t::set_interface_input_size:
            os << "set_int_in_size\n";
            break;

        case opcode_t::set_interface_output_size:
            os << "set_int_out_size\n";
            break;

        case opcode_t::operator_add:      os << "add\n";      break;
        case opcode_t::operator_subtract: os << "subtract\n"; break;
        case opcode_t::operator_multiply: os << "multiply\n"; break;
        case opcode_t::operator_divide:   os << "divide\n";   break;
        case opcode_t::operator_assign:   os << "assign\n";   break;
        //case opcode_t::operator_get_field:
        case opcode_t::operator_cmp_lt: os << "cmp_lt\n"; break;
        case opcode_t::operator_cmp_le: os << "cmp_le\n"; break;
        case opcode_t::operator_cmp_gt: os << "cmp_gt\n"; break;
        case opcode_t::operator_cmp_ge: os << "cmp_ge\n"; break;

        case opcode_t::push_new_local_integer: { // <opc> <ref>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_new_local_integer [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        case opcode_t::push_new_local_uinteger: { // <opc> <ref>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_new_local_uinteger [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        case opcode_t::push_new_local_string: { // <opc> <ref>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_new_local_string [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        case opcode_t::push_new_local_vector: { // <opc> <ref>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_new_local_vector [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        default:
            os << "UNKNOWN OPCODE\n"; break;
        }
    }
}

static opcode_t dis_get_opcode(
        std::vector<uint8_t>::iterator& iter) {

    union {
        uint16_t opc;
        size_t   ref;
        uint8_t u8[2];
    } u;

    u.u8[0] = *iter++;
    u.u8[1] = *iter++;

    return (opcode_t)u.opc;
}

static std::string dis_get_ref_name(
        struct module_desc_t* modptr,
        size_t ref) {

    if(ref >= modptr->constants.size()) {
        return "INVALID_REFERENCE";
    }

    return modptr->constants[ref];
}

static size_t dis_get_ref(
        std::vector<uint8_t>::iterator& iter,
        std::vector<uint8_t>::iterator& end) {

    size_t ref = 0ul;

    while(iter < end) {
        size_t u8 = (size_t)*iter++;
        ref <<= 7;
        ref |= (u8 & 0x7F);

        if(!(u8 & 0b10000000))
            return ref;
    }

    INTERNAL_ERR();
}