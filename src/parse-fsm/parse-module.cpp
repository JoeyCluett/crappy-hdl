#include "parse-module.h"
#include "src/error-util.h"

#include "src/hdl-module.h"
#include "src/hdl-runtime.h"
#include "src/hdl-variant.h"

static const int state_expect_module_name                = 0;
static const int state_module_header                     = 1;
static const int state_module_header_empty_expect_assign = 2;
static const int state_module_header_empty_expect_empty  = 3;
static const int state_module_header_empty_expect_semic  = 4;
static const int state_module_header_empty_expect_start  = 5;
static const int state_module_body                       = 6;

static int state_current = state_expect_module_name;

LexerToken_t module_name;
hdl_module_t* module_ptr;

bool parse_module(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename) {

    switch(state_current) {
    case state_expect_module_name:
        if(token.type == LexerToken_VarName) {
            module_name = token;
            module_ptr = new hdl_module_t;
            module_ptr->name = lexer_token_value(token, src);
            module_ptr->header_status = module_header_unknown;

            // add module to runtime
            hdl_runtime_add_module(rt, module_ptr);

            state_current = state_module_header;
            token_iter++;
            return false;

            // TODO : determine if module already exists.
            // throw error if already exists
        }
        else {
            throw_parse_error(
                "Expecting token of type [" + lexer_token_name(LexerToken_VarName) + 
                "]. Recieved token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_module_header:
        if(parse_module_header(rt, token, token_iter, src, filename, module_ptr)) {
            state_current = state_module_body;
            return false;
        }
        break;

    case state_module_body:
        token_iter++;
        return false;

        if(token.type == LexerToken_KW_start) {
            //token_iter++; // nexted body parser is expecting 'start' to be first token
            return false;
        }
        else {
            throw_parse_error(
                "Expecting 'start'. Received token of type [" + lexer_token_name_and_value(token, src) + "].",
                filename, src, token.start, token);
        }
        break;

    }

}

