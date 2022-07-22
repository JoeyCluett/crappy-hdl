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
#include <src/parse-fsm/parse-util.h>

//
// table with operator precedence
//
static const std::map<const int, const int> precedence_table = {
    { LexerToken_Syntax_Period,  1000 }, // 1000 as highest precedence chosen arbitrarily

    { LexerToken_Syntax_Invert,        801 }, // ~
    { LexerToken_Syntax_Not,           801 }, // !
    { LexerToken_Syntax_UnaryNegative, 801 }, // -

    { LexerToken_Syntax_Star,   660 }, // * and / have same precedence, left-right associativity
    { LexerToken_Syntax_Divide, 660 },

    { LexerToken_Syntax_Plus,      650 }, // + and - have same precedence, left-right associativity
    { LexerToken_Syntax_Minus,     650 },

    { LexerToken_Syntax_Equiv,     540 }, // == and != have same precedence, semantically invalid if multiple appear in same expression
    { LexerToken_Syntax_NotEquiv,  540 },

    { LexerToken_Syntax_Ampersand, 500 }, // & (AND)
    { LexerToken_Syntax_Caret,     499 }, // ^ (XOR)
    { LexerToken_Syntax_Pipe,      498 }, // | (OR)

    { LexerToken_Syntax_Assign,    50 }, // = (lowest priority operator)
};

static void parse_shunt_util_token_oq(
        std::vector<shunting_token_t>& output_queue, 
        LexerToken_t tok, 
        shunting_token_type_t tok_type,
        const std::vector<char>& src,
        const std::string& filename);

static void parse_shunt_util_token_operator_oq(
        std::vector<shunting_token_t>& output_queue, 
        std::stack<shunting_token_t>& work_stack, 
        LexerToken_t tok, 
        shunting_token_type_t tok_type,
        const std::vector<char>& src,
        const std::string& filename);

static void parse_shunt_push_output_queue(std::vector<shunting_token_t>& output_queue, shunting_token_t& stok);

static const int state_default = 0;

static int state_current = state_default;

bool parse_shunt(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr,
        std::stack<shunting_token_t>& work_stack,
        std::vector<shunting_token_t>& output_queue,
        const int flags) {

    shunting_token_t stok;

    if(parse_util_is_operator(token)) {

        if(token.type == LexerToken_Syntax_Minus) {
            if(parse_util_minus_is_unary(token_iter)) {
                parse_shunt_util_token_operator_oq(output_queue, work_stack, token, shunting_token_type_unary_operator, src, filename);
            }
            else {
                parse_shunt_util_token_operator_oq(output_queue, work_stack, token, shunting_token_type_binary_operator, src, filename);
            }
        }

        return false;
    }

    if(state_current == state_default) {
        switch(token.type) {
        case LexerToken_VarName:
            // could be field reference or local reference (figure it out)
            if(output_queue.size() == 0ul) {
                // local reference
                parse_shunt_util_token_oq(output_queue, token, shunting_token_type_local_reference, src, filename);
            }
            else {
                if(output_queue.back().token.type == LexerToken_Syntax_Period) {
                    // field reference
                    parse_shunt_util_token_oq(output_queue, token, shunting_token_type_field_reference, src, filename);
                }
                else {
                    // local reference
                    parse_shunt_util_token_oq(output_queue, token, shunting_token_type_local_reference, src, filename);
                }
            }
            token_iter++;
            return false;

        case LexerToken_NumberDec:
        case LexerToken_NumberHex:
        case LexerToken_NumberBin:
            parse_shunt_util_token_oq(output_queue, token, shunting_token_type_constant, src, filename);
            token_iter++;
            return false;

        case LexerToken_StringLiteral:
            parse_shunt_util_token_oq(output_queue, token, shunting_token_type_string, src, filename);
            token_iter++;
            return false;

        case LexerToken_Syntax_LParen:
            stok.type = shunting_token_type_lparen_sentinal;
            stok.token = token;
            work_stack.push(stok);
            token_iter++;
            return false;

        case LexerToken_Syntax_LBracket:
            stok.type = shunting_token_type_lindex_sentinal;
            stok.token = token;
            work_stack.push(stok);
            token_iter++;
            return false;

        case LexerToken_Syntax_LBrace:
            stok.type = shunting_token_type_lrange_sentinal;
            stok.token = token;
            work_stack.push(stok);
            token_iter++;
            return false;

        case LexerToken_Syntax_Semicolon:
            break;

        }
    }
}

static void parse_shunt_util_token_oq(
        std::vector<shunting_token_t>& output_queue, 
        LexerToken_t tok, 
        shunting_token_type_t tok_type,
        const std::vector<char>& src,
        const std::string& filename) {

    shunting_token_t stok;
    stok.token = tok;
    stok.type = tok_type;

    if(tok_type == shunting_token_type_local_reference) {
        // upper-level function assumes this is a local reference
        // check to see if its a builtin function
        if(parse_util_token_is_function(src, tok)) {
            stok.type = shunting_token_type_builtin_function;
            stok.function_ident = parse_util_token_function_ident_fast(src, tok);
        }
    }

    output_queue.push_back(stok); // TODO : static analysis
}

static void parse_shunt_util_token_operator_oq(
        std::vector<shunting_token_t>& output_queue, 
        std::stack<shunting_token_t>& work_stack, 
        LexerToken_t tok, 
        shunting_token_type_t tok_type,
        const std::vector<char>& src,
        const std::string& filename) {

    // shunting yard algorithm involves moving operators from work stack to output queue
    // upon encoutering an operator, any higher-precedence operators on the stack are pushed into the output
    // the current operator is then placed on the stack

    // assume error checking for unary operators is already done
    shunting_token_t stok;
    stok.type = tok_type;
    stok.token = tok;

    if(tok_type == shunting_token_type_unary_operator) {
        if(tok.type == LexerToken_Syntax_Minus)
            tok.type = LexerToken_Syntax_UnaryNegative;
    }

    auto operator_iter = precedence_table.find(tok.type);
    if(operator_iter == precedence_table.end()) {
        // ERROR
        throw_parse_error(
                "unrecognized operator : " + lexer_token_name_and_value(tok, src), 
                filename, src, tok.start, tok);
    }

    const int operator_prec = operator_iter->second;
    while(work_stack.size() > 0ul) {
        auto& work_top = work_stack.top();
        if(parse_util_is_operator(work_top.token)) {
            auto work_iter = precedence_table.find(work_top.token.type);
            if(work_iter == precedence_table.end()) {
                // ERROR
                throw_parse_error(
                    "unrecognized operator : " + lexer_token_name_and_value(tok, src),
                    filename, src, work_top.token.start, work_top.token);
            }

            if(work_iter->second >= operator_prec) {
                output_queue.push_back(work_top);
                work_stack.pop();
            }
            else {
                break;
            }
        }
    }

    work_stack.push(stok);
}
