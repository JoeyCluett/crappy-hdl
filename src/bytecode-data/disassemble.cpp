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

static std::string dis_get_jump_target(
        struct module_desc_t* modptr,
        std::vector<uint8_t>::iterator& iter,
        std::vector<uint8_t>::iterator& end);

static opcode_t dis_get_opcode(
        std::vector<uint8_t>::iterator& iter);

void disassemble_bytecode(std::ostream& os, struct module_desc_t* modptr) {

    os << "\n\n";

    auto opc_iter = modptr->bytecode.begin();
    auto opc_end  = modptr->bytecode.end();

    while(opc_iter < opc_end) {
        os << "[" << ((size_t)(opc_iter - modptr->bytecode.begin())) << "] : ";

        switch(dis_get_opcode(opc_iter)) {
        case opcode_t::clear_stack: os << "clear_stack\n"; break;

        case opcode_t::jump_exe: { // <opc> <label>
            os << "jump [" << dis_get_jump_target(modptr, opc_iter, opc_end) << "]\n";
            break;
        }

        case opcode_t::jump_false: { // <opc> <label>
            os << "jump_false [" << dis_get_jump_target(modptr, opc_iter, opc_end) << "]\n";
            break;
        }

        case opcode_t::jump_true: { // <opc> <label>
            os << "jump_true [" << dis_get_jump_target(modptr, opc_iter, opc_end) << "]\n";
            break;
        }

        case opcode_t::push_true:  os << "push_true\n"; break;
        case opcode_t::push_false: os << "push_false\n"; break;

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

        case opcode_t::push_new_local_ref: {
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_new_local_ref [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        case opcode_t::push_local_ref: { // <opc> <ref>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_local [" << dis_get_ref_name(modptr, ref) << "]\n";
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

        case opcode_t::push_vec_args_sentinal: // <opc>
            os << "push_vec_args_sentinal\n";
            break;

        case opcode_t::set_interface_size: // <opc>
            os << "set_interface_size\n";
            break;

        case opcode_t::pop_scope:      os << "pop_scope\n";   break;
        case opcode_t::return_:        os << "return\n";      break;
        case opcode_t::push_scope_for: os << "push_scope_for\n"; break;
        case opcode_t::push_scope_if:  os << "push_scope_if\n";  break;

        case opcode_t::push_arr_sentinal: os << "push_arr_sentinal\n"; break;
        case opcode_t::function_call: {
            os << "function_call [";
            function_type_t ft = static_cast<function_type_t>(*opc_iter++);
            switch(ft) {
            case function_type_t::UNKNOWN: os << "UNKNOWN]\n"; break;
            case function_type_t::push:    os << "push]\n";    break;
            case function_type_t::last:    os << "last]\n";    break;
            case function_type_t::print:   os << "print]\n";   break;
            case function_type_t::cast:    os << "cast]\n";    break;
            case function_type_t::cmpeq:   os << "cmpeq]\n";   break;
            case function_type_t::match:   os << "match]\n";   break;
            case function_type_t::decoder: os << "decoder]\n"; break;
            case function_type_t::vector:  os << "vector]\n";  break;
            case function_type_t::not_:    os << "not]\n";     break;
            case function_type_t::signal:  os << "signal]\n";  break;

            case function_type_t::and_     : os << "and]\n";       break;
            case function_type_t::nand     : os << "nand]\n";      break;
            case function_type_t::or_      : os << "or]\n";        break;
            case function_type_t::nor_     : os << "nor]\n";       break;
            case function_type_t::xor_     : os << "xor]\n";       break;
            case function_type_t::xnor_    : os << "xnor]\n";      break;
            case function_type_t::flipflop : os << "flipflop]\n";  break;
            case function_type_t::set_ff_data : os << "set_ff_data]\n";  break;
            case function_type_t::set_ff_clock: os << "set_ff_clock]\n"; break;
            case function_type_t::size     : os << "size]\n";      break;

            default: os << "invalid function type]\n"; break;
            }
            break;
        }

        case opcode_t::index_call: os << "index_call\n"; break;

        case opcode_t::operator_get_field:    os << "get_field\n"; break;
        case opcode_t::operator_unary_negate: os << "negate\n";    break;
        case opcode_t::operator_binary_not:   os << "not\n";       break;

        case opcode_t::operator_binary_and: os << "binary_and\n"; break;
        case opcode_t::operator_binary_xor: os << "binary_xor\n"; break;
        case opcode_t::operator_binary_or:  os << "binary_or\n";  break;

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

        case opcode_t::operator_range_desc: os << "range\n"; break;

        case opcode_t::push_new_local_any: {
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_new_local_any [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

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

        case opcode_t::module_call: { // <opc> <ref>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "module_call [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        case opcode_t::push_module_args_sentinal:
            os << "push_module_args_sentinal\n";
            break;

        case opcode_t::push_new_local_module: {
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_new_local_module [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        case opcode_t::push_bit_literal: { // <opc> <ref>
            size_t ref = dis_get_ref(opc_iter, opc_end);
            os << "push_bit_literal [" << dis_get_ref_name(modptr, ref) << "]\n";
            break;
        }

        default:
            os << "UNKNOWN OPCODE\n"; break;
        }
    }
}

static opcode_t dis_get_opcode(
        std::vector<uint8_t>::iterator& iter) {

    uint16_t u16 = 0;
    while(1) {
        uint8_t u = *iter++;
        u16 = (u16 << 7) | (u & 0x7F);

        if(!(u & 0x80)) {
            break;
        }
    }

    return static_cast<opcode_t>(u16);
}

static std::string dis_get_jump_target(
        struct module_desc_t* modptr,
        std::vector<uint8_t>::iterator& iter,
        std::vector<uint8_t>::iterator& end) {

    const size_t imag_ref = dis_get_ref(iter, end);
    const size_t target = modptr->jump_targets.at(imag_ref);

    if(target == ~0ul)
        return "UNDEFINED";
    return std::to_string(target);
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
