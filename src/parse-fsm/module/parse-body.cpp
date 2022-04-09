#include "parse-body.h"

#include <iostream>
#include <vector>
#include <string>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>

static const int state_expect_start = 0;
static const int state_default      = 1;

int state_current = state_expect_start;

bool parse_module_body(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr) {

    std::cout << "parse_module_body : " << lexer_token_name_and_value(token, src) << std::endl;

    switch(state_current) {
    case state_expect_start:
        if(token.type == LexerToken_KW_start) {

            std::cout << "    state_expect_start : start" << std::endl;

            state_current = state_default;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting 'start'. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_default:
        if(token.type == LexerToken_KW_end) {
            std::cout << "    state_default : module end" << std::endl;
            token_iter++;
            return true;
        }
        else {
            token_iter++;
            return false;
        }
        break;

    }


}

