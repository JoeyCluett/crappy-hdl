#include <src/semantic-analysis/syard.h>
#include <src/bytecode-data/opcodes.h>
#include <src/bytecode-data/disassemble.h>
#include <src/lexer.h>
#include <src/error-util.h>

#include <set>
#include <map>
#include <iostream>
#include <functional>

enum assoc_t {
    assoc_left_to_right,
    assoc_right_to_left,
};

//typedef std::pair<int, assoc_t> assoc_entry_t;

struct assoc_entry_t {
    int     first;
    assoc_t second;
    std::function<void(struct module_desc_t*)>
            third;
};

static const std::map<token_type_t, struct assoc_entry_t> precedence_table = {

    { token_type_t::period,         { 60, assoc_left_to_right, opc::operator_::get_field }}, // .

    { token_type_t::unary_negative, { 45, assoc_right_to_left, opc::operator_::unary_negate }}, // -
    { token_type_t::invert,         { 45, assoc_right_to_left, opc::operator_::binary_not }},   // ~

    { token_type_t::divide,         { 40, assoc_left_to_right, opc::operator_::divide   }}, // /
    { token_type_t::star,           { 40, assoc_left_to_right, opc::operator_::multiply }}, // *
    { token_type_t::minus,          { 35, assoc_left_to_right, opc::operator_::subtract }}, // -
    { token_type_t::plus,           { 35, assoc_left_to_right, opc::operator_::add      }}, // +

    { token_type_t::less_than,      { 30, assoc_left_to_right, opc::operator_::cmp_lt }}, // <
    { token_type_t::less_eq,        { 30, assoc_left_to_right, opc::operator_::cmp_le }}, // <=
    { token_type_t::greater_than,   { 30, assoc_left_to_right, opc::operator_::cmp_gt }}, // >
    { token_type_t::greater_eq,     { 30, assoc_left_to_right, opc::operator_::cmp_ge }}, // >=

    { token_type_t::equiv,          { 25, assoc_left_to_right, opc::UNIMPLEMENTED }}, // ==
    { token_type_t::not_equiv,      { 25, assoc_left_to_right, opc::UNIMPLEMENTED }}, // !=

    { token_type_t::ampersand,      { 20, assoc_left_to_right, opc::operator_::binary_and }}, //  &
    { token_type_t::caret,          { 15, assoc_left_to_right, opc::operator_::binary_xor }}, //  ^
    { token_type_t::pipe,           { 10, assoc_left_to_right, opc::operator_::binary_or  }}, //  |
    { token_type_t::colon,          {  7, assoc_left_to_right, opc::operator_::range_desc }}, // begin:end, [begin, end)
    { token_type_t::assign,         {  5, assoc_right_to_left, opc::operator_::assign }}, //  =
};

static const bool is_unary_operator(token_type_t t);

static const bool is_binary_operator(token_type_t t);

const bool token_is_operator(token_type_t t) {
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
        shunt_behavior_t shunt_behavior) {

    while(titer < tend) {
        token_t& tok = *titer++;

        //std::cout << "op stack size : " << shunt_stack.op_stack.size() << std::endl;
        
        if(shunt_behavior == shunt_behavior_before && end_types.find(tok.type) != end_types.end())
            return;

        //std::cout << lexer_token_desc(tok, p.src) << std::endl;
        //std::cout << "\nBEFORE EVAL\n\n";
        //shunting_yard_print_eval_stack(shunt_stack, p.src);

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wswitch-enum"
        switch(tok.type) {
        case token_type_t::comma:
            // seperator. evaluate operators on top of the stack
            while(shunt_stack.op_stack.size() > 0ul) {
                token_t c = shunt_stack.op_stack.back();
                if(token_is_operator(c.type)) {
                    shunting_yard_eval_operator(rtenv, modptr, p, titer, tend, shunt_stack, c);
                    precedence_table.at(c.type).third(modptr);
                    shunt_stack.op_stack.pop_back();
                } else {
                    break;
                }
            }
            break;

        case token_type_t::variable_name: {
            std::string val = lexer_token_value(tok, p.src);
            auto pr         = module_desc_get_idx_of_string(modptr, val);
            if(pr.first == false && (*(titer-2)).type != token_type_t::period)
                throw_parse_error("local variable with name `" + val + "' does not exist in module `" + modptr->name + "'", p.filename, p.src, tok);

            opc::push_local(modptr, pr.second);
            shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
            break;
        }

        case token_type_t::semicolon:
            shunting_yard_eval_semicolon(rtenv, modptr, p, titer, tend, shunt_stack);
            opc::clear_stack(modptr);
            break;

        case token_type_t::keyword_false_:
            opc::push_false(modptr);
            shunt_stack.eval_stack.push_back(eval_token_t::numeric_reference);
            break;

        case token_type_t::keyword_true_:
            opc::push_true(modptr);
            shunt_stack.eval_stack.push_back(eval_token_t::numeric_reference);
            break;

        case token_type_t::keyword_ref: {
            token_t local_name = *titer++;
            token_t& expect_assign = *titer;
            if(expect_assign.type != token_type_t::assign)
                throw_parse_error(
                        "Expecting `=', found " + lexer_token_desc(expect_assign, p.src),
                        p.filename, p.src, expect_assign);

            size_t local_idx = module_desc_add_string_constant(modptr, lexer_token_value(local_name, p.src));
            opc::push_new_local_ref(modptr, local_idx);
            shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
            break;
        }

        case token_type_t::keyword_local: // local declaration/definition
        {
            // local varname = ...
            // local varname : typespec;
            // local varname : typespec = ...;

            token_t& varname = *titer++;
            if(varname.type != token_type_t::variable_name)
                throw_parse_error("Expecting variable name, found " + lexer_token_desc(varname, p.src),
                        p.filename, p.src, varname);

            size_t varname_idx = module_desc_add_string_constant(modptr, lexer_token_value(varname, p.src));

            token_t& assign_or_colon = *titer++;
            if(assign_or_colon.type == token_type_t::assign) {
                opc::push_new_local_any(modptr, varname_idx);
                shunt_stack.op_stack.push_back(assign_or_colon); // assign operator
                shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
                break;
            } else if(assign_or_colon.type == token_type_t::semicolon) {
                // local varname; <-- this is an error
                throw_parse_error("Cannot default intialize local without explicit type", p.filename, p.src, varname);                
            } else if(assign_or_colon.type != token_type_t::colon) {
                throw_parse_error("Expecting `=' or `:', found " + lexer_token_desc(assign_or_colon, p.src),
                        p.filename, p.src, assign_or_colon);
            }

            token_t& typespec = *titer++;

            if(typespec.type == token_type_t::keyword_integer) {         opc::push_new_local_integer(modptr, varname_idx);
            } else if(typespec.type == token_type_t::keyword_uinteger) { opc::push_new_local_uinteger(modptr, varname_idx);
            } else if(typespec.type == token_type_t::keyword_string) {   opc::push_new_local_string(modptr, varname_idx);
            } else if(typespec.type == token_type_t::keyword_vector) {   opc::push_new_local_vector(modptr, varname_idx);
            } else {
                throw_parse_error(
                        "Expecting type specifier, found " + lexer_token_desc(typespec, p.src),
                        p.filename, p.src, typespec);
            }

            shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);            

            token_t& semic_or_assign = *titer++;

            if(semic_or_assign.type == token_type_t::semicolon) {
                titer--;
                break; // let other parts of shunting yard handle semicolon
            } else if(semic_or_assign.type != token_type_t::assign) {
                throw_parse_error("Expecting `;' or `=', found " + lexer_token_desc(semic_or_assign, p.src), p.filename, p.src, semic_or_assign);
            }

            titer--; // shunting yard will handle assignment operator
            break;
        }

        case token_type_t::keyword_vector: {
            token_t expect_lparen = *titer++;
            if(expect_lparen.type != token_type_t::lparen) {
                throw_parse_error("Expecting `(', found " + lexer_token_desc(expect_lparen, p.src), p.filename, p.src, expect_lparen);
            }

            shunt_stack.op_stack.push_back(tok);
            opc::push_vec_args_sentinal(modptr);
            shunt_stack.eval_stack.push_back(eval_token_t::function_arg_sentinal);

            break;
        }

        case token_type_t::keyword_module: {
            if(shunt_stack.eval_stack.size() == 0ul || shunt_stack.op_stack.size() == 0ul)
                throw_parse_error(
                        "Module instance must be part of assignment to local",
                        p.filename, p.src, tok);

            token_t& expect_dot = *titer++;
            if(expect_dot.type != token_type_t::period)
                throw_parse_error("Expecting `.', found " + lexer_token_desc(expect_dot, p.src),
                        p.filename, p.src, expect_dot);

            token_t& module_name = *titer++;

            if(module_name.type != token_type_t::variable_name)
                throw_parse_error("Expecting module name, found " + lexer_token_desc(module_name, p.src), p.filename, p.src, module_name);

            token_t& expect_lparen = *titer++;

            if(expect_lparen.type != token_type_t::lparen)
                throw_parse_error("Expecting `(', found " + lexer_token_desc(expect_lparen, p.src), p.filename, p.src, expect_lparen);

            opc::push_module_args_sentinal(modptr);

            module_name.type = token_type_t::module_ref;
            shunt_stack.op_stack.push_back(module_name);
            shunt_stack.eval_stack.push_back(eval_token_t::module_reference);

            break;
        }

        case token_type_t::keyword_in:
        case token_type_t::keyword_out:
        {
            token_t& expect_dot = *titer++;
            if(expect_dot.type != token_type_t::period)
                throw_parse_error("Expecting `.', found " + lexer_token_desc(expect_dot, p.src),
                        p.filename, p.src, expect_dot);

            token_t& in_name = *titer++;
            if(in_name.type != token_type_t::variable_name)
                throw_parse_error("Expecting variable name, found " + lexer_token_desc(in_name, p.src),
                        p.filename, p.src, in_name);

            size_t idx = module_desc_add_string_constant(modptr, lexer_token_value(in_name, p.src));
            tok.type == token_type_t::keyword_in ?
                    opc::push_in_ref(modptr, idx) :
                    opc::push_out_ref(modptr, idx);

            shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
            break;
        }

        case token_type_t::lbracket:
            opc::push_arr_sentinal(modptr);
            shunt_stack.eval_stack.push_back(eval_token_t::arr_access_sentinal);
            shunt_stack.op_stack.push_back(tok);
            break;

        case token_type_t::number_bin:
        case token_type_t::number_dec:
        case token_type_t::number_hex:
            opc::push_uinteger(modptr, lexer_token_to_uinteger(tok, p));
            shunt_stack.eval_stack.push_back(eval_token_t::numeric_reference);
            break;

        case token_type_t::bit_literal: {
            size_t idx = module_desc_add_string_constant(modptr, lexer_token_value(tok, p.src));
            opc::push_bit_literal(modptr, idx);
            shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
            break;
        }

        case token_type_t::lparen:
            shunt_stack.op_stack.push_back(tok);
            break;

        case token_type_t::function: {
            shunt_stack.op_stack.push_back(tok);
            opc::push_fn_args_sentinal(modptr);
            token_t& next_token = *titer++;
            if(next_token.type != token_type_t::lparen)
                throw_parse_error("Expecting '(', found " + lexer_token_desc(next_token, p.src), p.filename, p.src, next_token);
            shunt_stack.eval_stack.push_back(eval_token_t::function_arg_sentinal);
            break;
        }

        case token_type_t::rparen: {
            while(shunt_stack.op_stack.size() > 0ul) {
                token_t t = shunt_stack.op_stack.back();
                if(token_is_operator(t.type)) {
                    shunting_yard_eval_operator(rtenv, modptr, p, titer, tend, shunt_stack, t);
                    auto iter = precedence_table.find(t.type);
                    if(iter == precedence_table.end()) {
                        throw_parse_error(
                                "INTERNAL ERROR : OPERATOR " + lexer_token_desc(t, p.src) + 
                                    " MISSING FROM PRECENDENCE TABLE",
                                p.filename, p.src, t);
                    } else {
                        iter->second.third(modptr); // <-- generate opcode
                    }
                    shunt_stack.op_stack.pop_back();
                } else {
                    break;
                }
            }

            if(shunt_stack.op_stack.size() == 0ul)
                throw_parse_error("Mismatched ')'", p.filename, p.src, tok);

            token_t t = shunt_stack.op_stack.back();
            switch(t.type) {
            case token_type_t::lparen:
                shunt_stack.op_stack.pop_back();
                break;

            case token_type_t::module_ref: {
                std::cout << "rparen matched to module_reference\n";

                string_t modulename = lexer_token_value(t, p.src);
                size_t mname_idx = module_desc_add_string_constant(modptr, modulename);

                opc::module_call(modptr, mname_idx);
                shunt_stack.op_stack.pop_back();

                // empty the eval_stack until we hit a module_reference
                while(shunt_stack.eval_stack.size() > 0ul) {
                    eval_token_t et = shunt_stack.eval_stack.back();
                    if(et == eval_token_t::module_reference) {
                        break;
                    }
                    shunt_stack.eval_stack.pop_back();
                }

                if(shunt_stack.eval_stack.size() == 0ul) {
                    throw_parse_error(
                            "Expecting module reference, found " + lexer_token_desc(t, p.src),
                            p.filename, p.src, t);
                } else {
                    shunt_stack.eval_stack.pop_back();
                    shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
                }

                break;
            }

            case token_type_t::function: {
                string_t fname = lexer_token_value(t, p.src);
                auto tup = lexer_token_is_function(fname);
                if(!std::get<0>(tup))
                    INTERNAL_ERR();

                opc::function_call(modptr, std::get<2>(tup));

                while(
                        shunt_stack.eval_stack.size() > 0ul &&
                        shunt_stack.eval_stack.back() != eval_token_t::function_arg_sentinal)
                {
                    shunt_stack.eval_stack.pop_back();
                }

                shunt_stack.op_stack.pop_back();
                shunt_stack.eval_stack.pop_back(); // remove the function args sentinal
                shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);

                break;
            }

            case token_type_t::keyword_vector: {
                opc::function_call(modptr, function_type_t::vector);
                shunt_stack.op_stack.pop_back();

                while(
                        shunt_stack.eval_stack.size() > 0ul &&
                        shunt_stack.eval_stack.back() != eval_token_t::function_arg_sentinal)
                {
                    // TODO : check types that we are discarding
                    shunt_stack.eval_stack.pop_back();
                }

                shunt_stack.eval_stack.pop_back(); // remove the function args sentinal
                shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
                break;
            }

            case token_type_t::lbrace:
            case token_type_t::lbracket:
                throw_parse_error("Unmatched " + lexer_token_desc(t, p.src), p.filename, p.src, t);

            default:
                if(token_is_operator(t.type)) {
                    shunting_yard_eval_operator(rtenv, modptr, p, titer, tend, shunt_stack, t);
                    shunt_stack.op_stack.pop_back();
                } else {
                    shunting_yard_print_eval_stack(shunt_stack, p.src);
                    throw_parse_error("Unknown type when evaluating closing parentheses " + lexer_token_desc(t, p.src), p.filename, p.src, t);
                }
                break;
            }

            break;
        }
        case token_type_t::rbracket: {
            bool check = true;
            while(shunt_stack.op_stack.size() > 0ul && check) {
                token_t t = shunt_stack.op_stack.back();
                switch(t.type) {
                case token_type_t::lbracket:
                    while(shunt_stack.eval_stack.size() > 1ul) {
                        if(shunt_stack.eval_stack.back() != eval_token_t::arr_access_sentinal) {
                            shunt_stack.eval_stack.pop_back();
                        } else {
                            break;
                        }
                    }

                    if(shunt_stack.eval_stack.size() <= 1ul || shunt_stack.eval_stack.back() != eval_token_t::arr_access_sentinal)
                        throw_parse_error("Unmatched `]'", p.filename, p.src, t);

                    shunt_stack.eval_stack.pop_back();
                    if(shunt_stack.eval_stack.back() != eval_token_t::variable_reference)
                        throw_parse_error("Unable to index non-array entity " + lexer_token_desc(t, p.src),
                                p.filename, p.src, t);

                    // leave variable reference on eval_stack

                    opc::operator_::index_call(modptr);
                    shunt_stack.op_stack.pop_back(); // remove lbracket from op_stack
                    check = false;
                    break;                    

                case token_type_t::lbrace:
                case token_type_t::lparen:
                    throw_parse_error("Unmatched " + lexer_token_desc(t, p.src), p.filename, p.src, t);

                case token_type_t::interface_ref:
                    //std::cout << "Found interface ref" << std::endl << std::flush;
                    opc::set_interface_size(modptr);
                    shunt_stack.op_stack.pop_back();
                    break;

                default:
                    if(token_is_operator(t.type)) {
                        shunting_yard_eval_operator(rtenv, modptr, p, titer, tend, shunt_stack, t);
                        precedence_table.at(t.type).third(modptr);
                        shunt_stack.op_stack.pop_back();
                    } else {
                        throw_parse_error("Unknown type when evaluating closing bracket " + lexer_token_desc(t, p.src), p.filename, p.src, t);
                    }
                    break;
                }
            }
            break;
        }
        default:
            if(!token_is_operator(tok.type))
                throw_parse_error(
                        "[DEFAULT, syard] unknown token " + lexer_token_desc(tok, p.src),
                        p.filename, p.src, tok);

            //std::cout << "operator : " << lexer_token_desc(tok, p.src) << std::endl;

            auto cur_prec = precedence_table.find(tok.type);
            if(cur_prec == precedence_table.end())
                throw_parse_error("Unknown operator " + lexer_token_desc(tok, p.src), p.filename, p.src, tok);

            while(shunt_stack.op_stack.size() > 0ul) {
                token_t t = shunt_stack.op_stack.back();
                if(
                        t.type == token_type_t::lparen   ||
                        t.type == token_type_t::function ||
                        !token_is_operator(t.type))
                    break;

                auto t_prec = precedence_table.find(t.type);
                if(t_prec == precedence_table.end())
                    throw_parse_error("Unknown operator " + lexer_token_desc(t, p.src), p.filename, p.src, t);

                if(t_prec->second.first > cur_prec->second.first) {
                    shunting_yard_eval_operator(rtenv, modptr, p, titer, tend, shunt_stack, t);
                    t_prec->second.third(modptr); // <-- actual bytecode generation
                    shunt_stack.op_stack.pop_back();
                } else {
                    break;
                }
            }
            shunt_stack.op_stack.push_back(tok);
            break;
        }
        #pragma GCC diagnostic pop

        //std::cout << "\nAFTER EVAL\n\n";
        //shunting_yard_print_eval_stack(shunt_stack, p.src);
        //std::cout << ">>>> -------------------------------------------------------------\n";

        //shunting_yard_print_eval_stack(shunt_stack, p.src);

        if(shunt_behavior == shunt_behavior_after && end_types.find(tok.type) != end_types.end())
            return;
    }
}

void shunting_yard_eval_semicolon(
        struct runtime_env_t* rtenv,
        struct module_desc_t* modptr,
        struct parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend,
        shunting_stack_t& shunt_stack) {

    while(shunt_stack.op_stack.size() > 0) {
        token_t c = shunt_stack.op_stack.back();
        if(token_is_operator(c.type)) {
            shunting_yard_eval_operator(rtenv, modptr, p, titer, tend, shunt_stack, c);
            precedence_table.at(c.type).third(modptr);
            shunt_stack.op_stack.pop_back();
        } else {
            throw_parse_error("Expecting operator, found " + lexer_token_desc(c, p.src),
                    p.filename, p.src, c);
        }
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
        { token_type_t::colon,        opcode_t::operator_range_desc },
    };

    //std::cout << "eval operator " << lexer_token_desc(t, p.src) << std::endl;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch-enum"
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
    case token_type_t::colon:
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
    #pragma GCC diagnostic pop
}

void shunting_yard_print_eval_stack(shunting_stack_t& shunt_stack, src_t& s) {

    //std::cout << ">>>> -------------------------------------------------------------\n";
    std::cout << " eval stack (size=" << shunt_stack.eval_stack.size() << ")\n";
    std::cout << "========================\n";
    for(auto et : shunt_stack.eval_stack) {
        switch(et) {
        case eval_token_t::numeric_reference:     std::cout << "    numeric_reference\n"; break;
        case eval_token_t::variable_reference:    std::cout << "    variable_reference\n"; break;
        case eval_token_t::module_reference:      std::cout << "    module_reference\n"; break;
        case eval_token_t::left_paren:            std::cout << "    left_paren\n"; break;
        case eval_token_t::left_bracket:          std::cout << "    left_bracket\n"; break;
        case eval_token_t::function_arg_sentinal: std::cout << "    function_arg_list\n"; break;
        case eval_token_t::arr_access_sentinal:   std::cout << "    array_access_sentinal\n"; break;
        }
    }
    std::cout << "\n";

    std::cout << " op stack (size=" << shunt_stack.op_stack.size() << ")\n";
    std::cout << "========================\n";
    for(auto tok : shunt_stack.op_stack) {
        std::cout << "    " << lexer_token_desc(tok, s) << std::endl;
    }
    //std::cout << ">>>> -------------------------------------------------------------\n";
    std::cout << "\n\n";

}
