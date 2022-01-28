#include "parser.h"

#include "lexical-token.h"
#include "error-util.h"
#include "hdl-runtime.h"
#include "hdl-module.h"
#include "hdl-variant.h"

#include "parse-fsm/parse-global.h"
#include "parse-fsm/parse-module.h"

#include <vector>

HDL_Runtime_t* parse_analyze(
        const std::vector<LexerToken_t>& tokens, 
        const std::vector<char>& src,
        const std::string& filename) {

    HDL_Runtime_t* runtime = new HDL_Runtime_t;

    const int state_default = 0;
    const int state_require = 1;
    const int state_global  = 2;
    const int state_module  = 3;

    int state_current = state_default;

    auto token_iter      = tokens.begin();
    const auto token_end = tokens.end();

    while(token_iter != token_end) {
        const LexerToken_t token      = *token_iter;
        const std::string token_value = lexer_token_value(token, src);


        switch(state_current) {
        case state_default: // expecting one of 'requires', 'global', or 'module'
            if(token_value == "requires") {
                state_current = state_require;
                token_iter++;
            }
            else if(token_value == "global") {
                state_current = state_global;
                token_iter++;
            }
            else if(token_value == "module") {
                state_current = state_module;
                token_iter++;
            }
            else {
                // throw error
                throw_parse_error(
                    "Expecting one of 'requires', 'global', 'module'. Found token of type [" + 
                    lexer_token_name_and_value(token, src) + "]",
                    filename, src, token.start, token);
            }
            break;

        case state_require:
            if(token.type == LexerToken_VarName) {
                const LexerToken_t semicolon_token = *(token_iter + 1);
                if(semicolon_token.type == LexerToken_Syntax_Semicolon) {
                    // success
                    hdl_runtime_add_import_name(runtime, token_value);
                    token_iter += 2;
                    state_current = state_default;
                }
                else {
                    throw_parse_error(
                        "Expecting semicolon ';'. Recieved token of type [" + lexer_token_name_and_value(token, src) + "]",
                        filename, src, semicolon_token.start, semicolon_token);
                }
            }
            else {
                throw_parse_error(
                    "Expecting token of type " + lexer_token_name(LexerToken_VarName) + 
                    ". Recieved token of type [" + lexer_token_name_and_value(token, src) + "]",
                    filename, src, token.start, token);
            }
            break;

        case state_global:
            // global VariableName : datatype = constant;
            if(parse_global(runtime, token, token_iter, src, filename)) {
                state_current = state_default;
            }
            break;

        case state_module:
            if(parse_module(runtime, token, token_iter, src, filename)) {
                state_current = state_default;
            }
            break;
        }
    }

    return runtime;
}



