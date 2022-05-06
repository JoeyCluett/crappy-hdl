#include "parse-shunt.h"

#include <vector>
#include <string>
#include <map>
#include <stack>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>

//
// table with operator precedence
//
const std::map<const int, const int> precedence_table = {
    { LexerToken_Syntax_Period, 1000 }, // 1000 as highest precedence chosen arbitrarily

    { LexerToken_Syntax_Invert,    601 }, // ~ and ! have same precedence, left-right associativity
    { LexerToken_Syntax_Not,       601 }, // ... both have higher priority than any mathematical operator

    { LexerToken_Syntax_Plus,      550 }, // + and - have same precedence, left-right associativity
    { LexerToken_Syntax_Minus,     550 },

    { LexerToken_Syntax_Equiv,     540 }, // == and != have same precedence, semantically invalid if multiple appear in same expression
    { LexerToken_Syntax_NotEquiv,  540 },

    { LexerToken_Syntax_Ampersand, 500 }, // & (AND)
    { LexerToken_Syntax_Caret,     499 }, // ^ (XOR)
    { LexerToken_Syntax_Pipe,      498 }, // | (OR)

};

std::stack<LexerToken_t> work_stack;
std::vector<LexerToken_t> output_queue;

static const int state_default    = 0;
static const int state_global_ref = 1;

static int state_current = state_default;

bool parse_shunt(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr,
        const std::vector<const int>& end_types) {

    //
    // parses expressions with shunting yard algorithm
    //

    switch(state_current) {
    case state_default:
        if(token.type == LexerToken_KW_global) {
            //token_iter++;
            return false;
        }
        else if(token.type == LexerToken_KW_builtin) {
            state_current = state_global_ref;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error("unknown token type in parse_shunt : " + lexer_token_name_and_value(token, src), 
            filename, src, token.start, token);
        }
        break;



    }


}