#include "parse-inout-port.h"

#include <stdio.h>
#include <iostream>

static void handle_port(
    const LexerToken_t& port_name,
    const LexerToken_t& size_ref,
    const int port_io_type, 
    const int port_type,
    const std::vector<char>& src, 
    hdl_module_t* module_ptr,
    const std::string& filename);

static const int state_inout_type               = 0; // one of 'in_ports', 'out_ports', 'in', 'out'
static const int state_inout_expect_colon       = 1; // expecting ':'
static const int state_inout_port_name          = 2; // expecting a VarName type
static const int state_inout_port_name_followup = 3; // can be ',', ';', or '['
static const int state_inout_port_index_1st     = 4; // can be #constant, VarName, or 'global'
static const int state_inout_port_glb_dot       = 5; // expecting '.'
static const int state_inout_port_glb_varname   = 6; // expecting VarName
static const int state_inout_port_rbrkt         = 7; // expecting ']'
static const int state_inout_port_entry_end     = 8; // expecting ',' or ';'

static int state_current = state_inout_type;

static LexerToken_t port_name; // port name
static LexerToken_t size_ref;  // meaning depends on port_type
static int port_io_type;       // module_port_input, module_port_output
static int port_type;          // one of:
                               //   - module_port_type_bit             
                               //   - module_port_type_bitvector_const 
                               //   - module_port_type_bitvector_local 
                               //   - module_port_type_bitvector_global

bool parse_module_inout_port(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr) {

    //
    // valid in/out port declaration
    //
    // VarName;                 <-- implicitly single bit type
    // VarName[#constant];      <-- numeric constant used as bit vector size
    // VarName[VarName];        <-- use module argument as size spec
    // VarName[global.VarName]; <-- can use global as size spec
    //
    // multiple port declarations can go on the same line:
    //
    // VarNameA, VarNameB[32], VarNameC[global.CpuWidth]; <-- always terminated with a semicolon (this is how the parser identifies the end)
    //

    //std::cout << "- parse_module_inout_port : " << lexer_token_name_and_value(token, src) << std::endl;

    switch(state_current) {
    case state_inout_type:
        if(token.type == LexerToken_KW_in_ports) {
            port_io_type  = module_port_input;
            state_current = state_inout_expect_colon;
            token_iter++;
            return false;
        }
        else if(token.type == LexerToken_KW_out_ports) {
            port_io_type  = module_port_output;
            state_current = state_inout_expect_colon;
            token_iter++;
            return false;
        }
        else {
            // not even sure how this error would occur
            throw_parse_error(
                "Expecting one of 'in_ports', 'out_ports', 'in', or 'out'. Received token of type [" + 
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_expect_colon:
        if(token.type == LexerToken_Syntax_Colon) {
            state_current = state_inout_port_name;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting ':'. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_name:
        if(token.type == LexerToken_VarName) {
            port_name = token;
            state_current = state_inout_port_name_followup;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting variable name. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_name_followup: // expecting ',', ';', or '['
        if(token.type == LexerToken_Syntax_Comma) {
            handle_port(port_name, size_ref, port_io_type, module_port_type_bit, src, module_ptr, filename);
            token_iter++;
            state_current = state_inout_port_name;
            return false;
        }
        else if(token.type == LexerToken_Syntax_Semicolon) {
            handle_port(port_name, size_ref, port_io_type, module_port_type_bit, src, module_ptr, filename);
            token_iter++;
            state_current = state_inout_type;
            return true;
        }
        else if(token.type == LexerToken_Syntax_LBracket) {
            state_current = state_inout_port_index_1st;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting one of ',' or ';' or '['. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_index_1st: // expecting #constant, VarName, or 'global'
        if(lexer_token_is_number_type(token)) {
            size_ref = token;
            port_type = module_port_type_bitvector_const;
            token_iter++;
            state_current = state_inout_port_rbrkt;
            return false;
        }
        else if(token.type == LexerToken_VarName) {
            size_ref = token;
            port_type = module_port_type_bitvector_local;
            token_iter++;
            state_current = state_inout_port_rbrkt;
            return false;
        }
        else if(token.type == LexerToken_KW_global) {
            port_type = module_port_type_bitvector_global;
            token_iter++;
            state_current = state_inout_port_glb_dot;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting a variable name, numeric constant, or 'global'. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_glb_dot:
        if(token.type == LexerToken_Syntax_Period) {
            token_iter++;
            state_current = state_inout_port_glb_varname;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting '.'. Recieved token of type [" +
                lexer_token_name_and_value(token, src) + "]", 
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_glb_varname: // global.'varname'
        if(token.type == LexerToken_VarName) {
            size_ref = token;
            token_iter++;
            state_current = state_inout_port_rbrkt;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting variable name. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_rbrkt:
        if(token.type == LexerToken_Syntax_RBracket) {
            handle_port(port_name, size_ref, port_io_type, port_type, src, module_ptr, filename);
            token_iter++;
            state_current = state_inout_port_entry_end;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting ']'. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_entry_end: // expecting ',' or ';'
        if(token.type == LexerToken_Syntax_Comma) {
            state_current = state_inout_port_name;
            token_iter++;
            return false;
        }
        else if(token.type == LexerToken_Syntax_Semicolon) {
            state_current = state_inout_type;
            token_iter++;
            return true;
        }
        else {
            throw_parse_error(
                "Expecting one of ',' or ';'. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    default:
        throw std::runtime_error("Unknown internal error");
    }

}

static void handle_port(
        const LexerToken_t& port_name, 
        const LexerToken_t& size_ref,
        const int port_io_type, 
        const int port_type,
        const std::vector<char>& src, 
        hdl_module_t* module_ptr,
        const std::string& filename) {

//    {
//        const char* io_type_str[] = {
//            "input",
//            "output"
//        };
//
//        const char* type_str[] = {
//            "bit",
//            "bitvector_const",
//            "bitvector_local",
//            "bitvector_global",
//        };
//
//        const std::string str_port_name = lexer_token_value(port_name, src);
//
//        std::cout << "handle_port:\n";
//        std::cout << "    port name : " << str_port_name << "\n";
//        std::cout << "    I/O type  : " << io_type_str[port_io_type] << "\n";
//        std::cout << "    port type : " << type_str[port_type] << "\n\n" << std::flush;
//    }

    module_port_t mport;
    mport.io_type = port_io_type; // input, output
    mport.type    = port_type;    // bit, bitvector_const, bitvector_local, bitvector_global

    const std::string str_port_name = lexer_token_value(port_name, src);

    if(port_type == module_port_type_bit) {
        module_ptr->module_io_port_interface.insert({ lexer_token_value(port_name, src), mport });
    }
    else if(port_type == module_port_type_bitvector_const) {
        // find what the const value is
        size_t l = lexer_token_number_to_ulong(size_ref, src);
        mport.vector_size = l;
        module_ptr->module_io_port_interface.insert({ lexer_token_value(port_name, src), mport });
    }
    else if(port_type == module_port_type_bitvector_local) {
        // look through argument list and find which one this local refers to 
        const std::string val = lexer_token_value(size_ref, src);

        auto arg_iter = module_ptr->argument_map.find(val);
        if(arg_iter == module_ptr->argument_map.end()) 
            throw_parse_error(
                "Argument with name '" + val + "' not found in module '" + module_ptr->name + "'.",
                filename, src, port_name.start, port_name);

        mport.argument_index = arg_iter->second.second;
    }
    else if(port_type == module_port_type_bitvector_global) {
        
        // put global name in string array and reference location
        const size_t name_index = module_ptr->string_array.size();
        const std::string global_name = lexer_token_value(size_ref, src);
        module_ptr->string_array.push_back(global_name);

        mport.global_index = name_index;
    }

    // place module port in module
    module_ptr->module_io_port_interface.insert({ str_port_name, mport });
}
