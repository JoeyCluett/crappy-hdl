#include "parse-module.h"

#include <src/error-util.h>

#include <src/parse-fsm/module/parse-arg-list.h>
#include <src/parse-fsm/module/parse-header.h>
#include <src/parse-fsm/module/parse-body.h>

#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>

static const int state_expect_module_name                = 0;
static const int state_arg_list                          = 1;
static const int state_module_header                     = 2;
static const int state_module_header_empty_expect_assign = 3;
static const int state_module_header_empty_expect_empty  = 4;
static const int state_module_header_empty_expect_semic  = 5;
static const int state_module_header_empty_expect_start  = 6;
static const int state_module_body                       = 7;

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
        //std::cout << "parse-module : state_expect_module_name\n";
        if(token.type == LexerToken_VarName) {
            module_name = token;
            module_ptr = new hdl_module_t;
            module_ptr->name          = lexer_token_value(token, src);
            module_ptr->header_status = module_header_unknown;
            module_ptr->n_in_ports    = 0ul;
            module_ptr->n_out_ports   = 0ul;

            // add module to runtime
            const int result = hdl_runtime_add_module(rt, module_ptr);
            if(result == HDL_ADD_MODULE_ALREADY_EXISTS) {
                throw_parse_error(
                    "Module '" + module_ptr->name + "' already exists",
                    filename, src, token.start, token);
            }

            state_current = state_arg_list;
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

    case state_arg_list:
        //std::cout << "parse-module : state_arg_list\n";
        if(parse_arg_list(rt, token, token_iter, src, filename, module_ptr)) {

            // display contents of module argument list:
            module_arglist_print_info(module_ptr);

            state_current = state_module_header;
            return false;
        }
        break;

    case state_module_header:
        //std::cout << "parse-module : state_module_header\n";
        if(parse_module_header(rt, token, token_iter, src, filename, module_ptr)) {
            state_current = state_module_body;
            return false;
        }
        break;

    case state_module_body:
        //std::cout << "parse-module : state_module_body\n";

        if(parse_module_body(rt, token, token_iter, src, filename, module_ptr)) {
            // parse_module_body ends on 'end' keyword. dont advance token iterator
            state_current = state_expect_module_name;
            return true;
        }
        else {
            return false;
        }
        break;

/*        token_iter++;
        return false;

        if(token.type == LexerToken_KW_start) {
            //token_iter++; // nested body parser is expecting 'start' to be first token
            return false;
        }
        else {
            throw_parse_error(
                "Expecting 'start'. Received token of type [" + lexer_token_name_and_value(token, src) + "].",
                filename, src, token.start, token);
        }
        break;
*/
    }

    return false;
}

