#include <src/semantic-analysis/syard.h>
#include <src/bytecode-data/opcodes.h>
#include <src/lexer.h>
#include <src/error-util.h>

#include <set>
#include <map>
#include <iostream>

enum assoc_t {
    assoc_left_to_right,
    assoc_right_to_left,
};

typedef std::pair<int, assoc_t> assoc_entry_t;

static const std::map<token_type_t, assoc_entry_t> precedence_table = {

    { token_type_t::period,         { 60, assoc_left_to_right }}, // .

    { token_type_t::unary_negative, { 45, assoc_right_to_left }}, // -
    { token_type_t::invert,         { 45, assoc_right_to_left }}, // ~

    { token_type_t::divide,         { 40, assoc_left_to_right }}, // /
    { token_type_t::star,           { 40, assoc_left_to_right }}, // *

    { token_type_t::minus,          { 35, assoc_left_to_right }}, // -
    { token_type_t::plus,           { 35, assoc_left_to_right }}, // +

    { token_type_t::less_than,      { 30, assoc_left_to_right }}, // <
    { token_type_t::less_eq,        { 30, assoc_left_to_right }}, // <=
    { token_type_t::greater_than,   { 30, assoc_left_to_right }}, // >
    { token_type_t::greater_eq,     { 30, assoc_left_to_right }}, // >=

    { token_type_t::equiv,          { 25, assoc_left_to_right }}, // ==
    { token_type_t::not_equiv,      { 25, assoc_left_to_right }}, // !=

    { token_type_t::ampersand,      { 20, assoc_left_to_right }}, //  &
    { token_type_t::caret,          { 15, assoc_left_to_right }}, //  ^
    { token_type_t::pipe,           { 10, assoc_left_to_right }}, //  |

    { token_type_t::assign,         {  5, assoc_right_to_left }}, //  =
};

static const bool is_unary_operator(token_type_t t);

static const bool is_binary_operator(token_type_t t);

static const bool is_operator(token_type_t t) {
    return precedence_table.find(t) != precedence_table.end();
}

void process_shunting_yard(
        struct runtime_env_t* rtenv,
        struct module_desc_t* modptr,
        struct parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend,
        shunting_stack_t& shunt_stack,
        const std::set<token_type_t>& end_types,
        const size_t min_stack_size) {

    size_t stack_size = shunt_stack.op_stack.size();
    while(stack_size >= min_stack_size && titer < tend) {
        token_t& tok = *titer++;

        std::cout << "op stack size : " << shunt_stack.op_stack.size() << std::endl;
        std::cout << lexer_token_desc(tok, p.src) << std::endl;

        if(end_types.find(tok.type) != end_types.end())
            return;

        switch(tok.type) {
        case token_type_t::variable_name: {
            std::string val = lexer_token_value(tok, p.src);
            auto pr         = module_desc_get_idx_of_string(modptr, val);
            if(pr.first == false)
                throw_parse_error("local variable with name '" + val + "' does not exist in module '" + modptr->name + "'", p.filename, p.src, tok);

            opc.push_local_ref(modptr, pr.second);
            shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
            break;
        }

        case token_type_t::number_bin:
        case token_type_t::number_dec:
        case token_type_t::number_hex:
            opc.push_uinteger(modptr, lexer_token_to_uinteger(tok, p));
            shunt_stack.eval_stack.push_back(eval_token_t::numeric_reference);
            break;

        case token_type_t::lparen:
            shunt_stack.op_stack.push_back(tok);
            break;

        case token_type_t::function: {
            shunt_stack.op_stack.push_back(tok);
            opc.push_fn_args_sentinal(modptr);
            token_t& next_token = *titer++;
            if(next_token.type != token_type_t::lparen)
                throw_parse_error("Expecting '(', found " + lexer_token_desc(next_token, p.src), p.filename, p.src, next_token);
            shunt_stack.eval_stack.push_back(eval_token_t::function_arg_list);
            break;
        }

        case token_type_t::rparen:
            while(shunt_stack.op_stack.size() > 0ul) {
                token_t t = shunt_stack.op_stack.back();

                if(is_operator(t.type)) {
                    shunting_yard_eval_operator(rtenv, modptr, p, titer, tend, shunt_stack, t);
                } else if(t.type == token_type_t::lparen) {
                    shunt_stack.op_stack.pop_back();
                } else if(t.type == token_type_t::function) {
                    opc.push_fn_args_sentinal(modptr);
                }

            }
            break;
        }

        stack_size = shunt_stack.op_stack.size();
    }

}

void shunting_yard_eval_operator(
        struct runtime_env_t* rtenv,
        struct module_desc_t* modptr,
        struct parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend,
        shunting_stack_t& shunt_stack,
        const token_t& t) {

    static const std::map<token_type_t, opcode_t> opcode_map = {
        { token_type_t::period,       opcode_t::operator_get_field },
        { token_type_t::divide,       opcode_t::operator_divide    },
        { token_type_t::star,         opcode_t::operator_multiply  },
        { token_type_t::plus,         opcode_t::operator_add       },
        { token_type_t::minus,        opcode_t::operator_subtract  },
        { token_type_t::assign,       opcode_t::operator_assign    },
        { token_type_t::greater_than, opcode_t::operator_cmp_gt    },
        { token_type_t::greater_eq,   opcode_t::operator_cmp_ge    },
        { token_type_t::less_than,    opcode_t::operator_cmp_lt    },
        { token_type_t::less_eq,      opcode_t::operator_cmp_le    },
    };

    switch(t.type) {
    case token_type_t::period:
    {
        if(shunt_stack.eval_stack.size() < 2ul)
            throw_parse_error("Insufficient operands for operator " + lexer_token_desc(t, p.src), p.filename, p.src, t);

        eval_token_t field_ref = shunt_stack.eval_stack.back();
        shunt_stack.eval_stack.pop_back();
        if(field_ref != eval_token_t::variable_reference)
            throw_parse_error("Invalid field reference for operator " + lexer_token_desc(t, p.src), p.filename, p.src, t);
        eval_token_t module_ref = shunt_stack.eval_stack.back();
        shunt_stack.eval_stack.pop_back();
        if(module_ref != eval_token_t::variable_reference)
            throw_parse_error("Invalid module reference for operator " + lexer_token_desc(t, p.src), p.filename, p.src, t);

        shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
        break;
    }
    case token_type_t::unary_negative:
    case token_type_t::invert:
    {
        if(shunt_stack.eval_stack.size() < 1ul)
            throw_parse_error("No operand for operator " + lexer_token_desc(t, p.src), p.filename, p.src, t);

        eval_token_t ref = shunt_stack.eval_stack.back();
        if(ref != eval_token_t::variable_reference && ref != eval_token_t::numeric_reference)
            throw_parse_error("Invalid type for operator " + lexer_token_desc(t, p.src), p.filename, p.src, t);
        break;
    }
    case token_type_t::divide:
    case token_type_t::star:
    case token_type_t::minus:
    case token_type_t::plus:
    case token_type_t::less_than:
    case token_type_t::less_eq:
    case token_type_t::greater_than:
    case token_type_t::greater_eq:
    case token_type_t::equiv:
    case token_type_t::not_equiv:
    case token_type_t::ampersand:
    case token_type_t::caret:
    case token_type_t::pipe:
    case token_type_t::assign:
    {
        if(shunt_stack.eval_stack.size() < 2ul)
            throw_parse_error("Insufficient operands for operator " + lexer_token_desc(t, p.src), p.filename, p.src, t);
        
        eval_token_t last_eval = shunt_stack.eval_stack.back();
        shunt_stack.eval_stack.pop_back();
        if(last_eval != eval_token_t::numeric_reference && last_eval != eval_token_t::variable_reference)
            throw_parse_error("Invalid right-hand operand type for operator " + lexer_token_desc(t, p.src), p.filename, p.src, t);
        eval_token_t second_to_last = shunt_stack.eval_stack.back();
        shunt_stack.eval_stack.pop_back();
        if(second_to_last != eval_token_t::numeric_reference && second_to_last != eval_token_t::variable_reference)
            throw_parse_error("Invalid left-hand operand type for operator " + lexer_token_desc(t, p.src), p.filename, p.src, t);

        if(last_eval == second_to_last) {
            if(last_eval == eval_token_t::variable_reference) {
                shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
            } else {
                shunt_stack.eval_stack.push_back(eval_token_t::numeric_reference);
            }
        } else {
            shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
        }
        break;
    }
    default:
        INTERNAL_ERR();
    }
}
