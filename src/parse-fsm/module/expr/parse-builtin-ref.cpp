#include <vector>
#include <string>
#include <map>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>

static const int state_builtin_kw = 0; // 'builtin'.MODULE_TYPE(
static const int state_dot        = 1; // builtin'.'MODULE_TYPE(
static const int state_type_ref   = 2; // builtin.'MODULE_TYPE'(
static const int state_open_paren = 3; // builtin.MODULE_TYPE'('

static int state_current = state_builtin_kw;

bool parse_builtin_ref(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr) {

    //
    // parse builtin.MODULE_TYPE expressions
    //

    switch(state_current) {
    case state_builtin_kw:
        if(token.type == LexerToken_KW_builtin) {
            state_current = state_dot;
            token_iter++;
            return false;
        }
        else {
            // throw error
        }
        break;

    case state_dot:
        if(token.type == LexerToken_Syntax_Period) {
            state_current = state_type_ref;
            token_iter++;
            return false;
        }
        else {
            // throw error
        }
        break;

    case state_type_ref:
        if(token.type == LexerToken_VarName) {
            state_current = state_open_paren;
            token_iter++;
            return false;
        }
        else {
            // throw error
        }
        break;

    case state_open_paren:
        if(token.type == LexerToken_Syntax_LParen) {
            state_current = state_builtin_kw;
            token_iter++;
            return true;
        }
        else {
            // throw error
        }
        break;

    default:
        throw std::runtime_error("parse_builtin_ref : unknown internal error");
    }

}
