#include "parse-header.h"

#include <vector>
#include <string>
#include <iostream>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>
#include <src/parse-fsm/module/parse-inout-port.h> // need function and a few constants

// parsing is identical for each. just need to know which one it is
static int port_list_type;

static const int state_default = 0;
static const int state_module_header_empty_expect_assign = 1; // header '=' empty;
static const int state_module_header_empty_expect_empty  = 2; // header = 'empty';
static const int state_module_header_empty_expect_semic  = 3; // header = empty ';'
static const int state_module_header_empty_expect_start  = 4;
static const int state_handle_inout_port                 = 5;
static int state_current = state_default;

bool parse_module_header(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr) {

    //std::cout << "parse_module_header : " << lexer_token_name_and_value(token, src) << std::endl;

    switch(state_current) {
    case state_default:

        // need one of 'header', 'in_ports', 'out_ports', 'in', 'out'
        if(token.type == LexerToken_KW_header) { // 'header'

            if(module_ptr->header_status == module_header_not_empty) {
                // module cannot be both declared empty AND have io port declarations
                throw_parse_error(
                    "Module '" + module_ptr->name + "' header previously declared not empty",
                    filename, src, token.start, token);
            }
            else if(module_ptr->header_status == module_header_empty) {
                throw_parse_error(
                    "Module '" + module_ptr->name + "' header cannot be declared empty multiple times",
                    filename, src, token.start, token);
            }


            module_ptr->header_status = module_header_empty;
            state_current = state_module_header_empty_expect_assign;
            token_iter++;
            return false;
        }
        else if(token.type == LexerToken_KW_in_ports || token.type == LexerToken_KW_out_ports) {
            // 'in_ports', 'out_ports', 'in', or 'out'
            
            if(module_ptr->header_status == module_header_empty) {
                throw_parse_error(
                    "Module '" + module_ptr->name + "' header previously declared empty",
                    filename, src, token.start, token);
            }
            else {
                // in/out port is acceptable
                module_ptr->header_status = module_header_not_empty;
                state_current = state_handle_inout_port;

                // dont advance token iterator
                //token_iter++;

                return false;
            }
        }
        else if(token.type == LexerToken_KW_start) {
            if(module_ptr->header_status == module_header_unknown) {
                // module header cannot be empty
                throw_parse_error(
                    "Module '" + module_ptr->name + "' cannot be implicitly empty. "
                    "Use 'header=empty;' to explicitly specify empty header",
                    filename, src, token.start, token);
            }
            else {
                // do not advance token iterator
                return true;
            }
        }
        else {
            throw_parse_error(
                "Expecting one of 'header', 'in_ports', 'out_ports'. Received token of type [" + 
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_module_header_empty_expect_assign:
        if(token.type == LexerToken_Syntax_Assign) {
            state_current = state_module_header_empty_expect_empty;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting '='. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_module_header_empty_expect_empty:
        if(token.type == LexerToken_KW_empty) {
            state_current = state_module_header_empty_expect_semic;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting one of 'empty'. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_module_header_empty_expect_semic:
        if(token.type == LexerToken_Syntax_Semicolon) {
            state_current = state_module_header_empty_expect_start;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting ';'. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_module_header_empty_expect_start:
        if(token.type == LexerToken_KW_start) {
            
            //
            // dont advance token iterator. parse_module_body() expects 'start' to be the first token
            // token_iter++
            //

            state_current = state_default;
            return true;
        }
        else {
            throw_parse_error(
                "Expecting 'start'. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_handle_inout_port:

        if(parse_module_inout_port(rt, token, token_iter, src, filename, module_ptr)) {
            state_current = state_default;
        }
        return false;
    
    }
}
