#include "parse-shunt.h"

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stack>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>
#include <src/ir/opcodes.h>

//
// table with operator precedence
//
static const std::map<const int, const int> precedence_table = {
    { LexerToken_Syntax_Period,  1000 }, // 1000 as highest precedence chosen arbitrarily

    { LexerToken_Syntax_Invert,        801 }, // ~
    { LexerToken_Syntax_Not,           801 }, // !
    { LexerToken_Syntax_UnaryNegative, 801 }, // -

    { LexerToken_Syntax_Plus,      650 }, // + and - have same precedence, left-right associativity
    { LexerToken_Syntax_Minus,     650 },

    { LexerToken_Syntax_Equiv,     540 }, // == and != have same precedence, semantically invalid if multiple appear in same expression
    { LexerToken_Syntax_NotEquiv,  540 },

    { LexerToken_Syntax_Ampersand, 500 }, // & (AND)
    { LexerToken_Syntax_Caret,     499 }, // ^ (XOR)
    { LexerToken_Syntax_Pipe,      498 }, // | (OR)

    { LexerToken_Syntax_Assign,    50 }, // lowest priority operator

};

static const std::map<const std::string, uint16_t>
        module_builtin_table = {
    { "cmpeq",   0x0000 },
    { "cmpneq",  0x0001 },
    { "decoder", 0x0002 },
};

static const std::map<const std::string, Opcode_t>
        native_function_table = {
    { "print",   Opcode_FN_PRINT   },
    { "println", Opcode_FN_PRINTLN },
    { "max",     Opcode_FN_MAX     },
    { "min",     Opcode_FN_MIN     },
};

static std::stack<shunting_token_t>  work_stack;
static std::vector<shunting_token_t> output_queue;

static LexerToken_t first_token = { LexerToken_UNKNOWN, -1, -1 };
static shunting_token_t shunt;
static std::vector<LexerToken_t>::const_iterator first_token_iter;

static void handle_operator(const LexerToken_t& tok);
static const bool is_operator(const LexerToken_t& tok);
static const bool is_literal_type(const LexerToken_t& tok);

static const int state_first_token = 0;
static const int state_default     = 1;
static const int state_local_decl  = 2;
static const int state_builtin_ref = 3;
static const int state_global_ref  = 4;

static int state_current = state_default;

bool parse_shunt(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr) {

    //
    // parses expressions with shunting yard algorithm
    //

    switch(state_current) {
    case state_first_token:
        first_token      = token;
        first_token_iter = token_iter;
        state_current    = state_default;
        return false;

    case state_default:

        switch(token.type) {
        case LexerToken_KW_local:
            state_current = state_local_decl;
            return false;

        case LexerToken_KW_builtin:
            state_current = state_builtin_ref;
            return false;

        case LexerToken_KW_global:
            state_current = state_global_ref;
            return false;

        case LexerToken_Syntax_Semicolon:
            switch(work_stack.size()) {
            case 0ul: // empty
                state_current = state_first_token;
                return true; break;
            case 1ul: // 1 element needs to be assignment operator
                if(
                        work_stack.top().type != shunting_token_type_operator || 
                        work_stack.top().token.type != LexerToken_Syntax_Assign) {
                    // one of the two types needed for this to be an assignment operator dont match up.
                    // expression is malformed
                    throw_parse_error(
                            "malformed expression (possibly missing assignment) starting at:", 
                            filename, src, first_token.start, first_token);
                }
                else {
                    output_queue.push_back(work_stack.top());
                    work_stack.pop();
                    state_current = state_first_token;
                    return true; break;
                }
                break;
            default:
                throw_parse_error(
                        "malformed expression starting at:", 
                        filename, src, first_token.start, first_token);
            }
            break;

        // special operator. can be unary or binary
        case LexerToken_Syntax_Minus:
            {
                const LexerToken_t& previous_token = *(token_iter - 1);
                if(
                        token_iter == first_token_iter ||
                        is_operator(previous_token)    ||
                        previous_token.type == LexerToken_Syntax_LParen) {
                    shunt.type  = shunting_token_type_unary_operator;
                    shunt.token = token;
                    shunt.token.type = LexerToken_Syntax_UnaryNegative;
                    work_stack.push(shunt);
                    return false;
                }
                else {
                    if(
                            is_literal_type(previous_token)                 || 
                            previous_token.type == LexerToken_Syntax_RParen ||
                            previous_token.type == LexerToken_VarName) {
                        shunt.type = shunting_token_type_operator;
                        shunt.token = token;
                        work_stack.push(shunt);
                        return false;
                    }
                }
            }
            break;

        // list of operators. precedence determined by precedence_table above
        case LexerToken_Syntax_Not:    // always unary
        case LexerToken_Syntax_Invert: // ...
        case LexerToken_Syntax_Plus:
        case LexerToken_Syntax_Equiv:
        case LexerToken_Syntax_NotEquiv:
        case LexerToken_Syntax_Ampersand:
        case LexerToken_Syntax_Caret:
        case LexerToken_Syntax_Pipe:

            return false;

        // special type of operator, must be local references before and after
        case LexerToken_Syntax_Period:
            if(token_iter == first_token_iter) {
                // throw error. cant be first token in expression
                return false;
            }
            else {

            }
            break;

        // numbers, bit-literals, and strings
        case LexerToken_NumberBin: case LexerToken_NumberDec: case LexerToken_NumberHex:
        case LexerToken_BitLiteralMulti: case LexerToken_BitLiteralSingle:
        case LexerToken_StringLiteral:
            // push into output queue
            shunt.type = shunting_token_type_constant;
            shunt.token = token;
            output_queue.push_back(shunt);
            return false;

        default:
            throw_parse_error("unknown token type in parse_shunt : " + lexer_token_name_and_value(token, src), 
            filename, src, token.start, token);            
            break;
        }
        break;

    case state_local_decl:
    case state_builtin_ref:
    case state_global_ref:
        std::cout << "local-decl, builtin-ref, or global-ref\n";
        token_iter++;
        return false;

    }


}

static const bool is_literal_type(const LexerToken_t& tok) {
    switch(tok.type) {
    case LexerToken_NumberBin: case LexerToken_NumberDec: case LexerToken_NumberHex:
    case LexerToken_BitLiteralMulti: case LexerToken_BitLiteralSingle:
    case LexerToken_StringLiteral:
        return true;
    }
    return false;
}

static const bool is_operator(const LexerToken_t& tok) {
    switch(tok.type) {
    
    }
}

static void handle_operator(const LexerToken_t& tok) {
    return;
}
