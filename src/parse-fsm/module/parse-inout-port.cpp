#include "parse-inout-port.h"

#include <stdio.h>
#include <iostream>

static void handle_port_single_bit(
    const int port_type, 
    const LexerToken_t& port_name, 
    const std::vector<char>& src, 
    hdl_module_t* module_ptr,
    const std::string& filename);

static void handle_port_bitvector(
    const int port_type,
    const size_t port_size,
    const LexerToken_t& port_name, 
    const std::vector<char>& src, 
    hdl_module_t* module_ptr,
    const std::string& filename);

static void dbg_print_ports(hdl_module_t* module_ptr, const std::vector<char>& src);

static const int state_inout_port_name          = 0; // expecting a VarName type
static const int state_inout_port_name_followup = 1; // can be ',', ';', or '['
static const int state_inout_port_index_1st     = 2; // can be #constant, VarName, or 'global'
static const int state_inout_port_glb_dot       = 3; // expecting '.'
static const int state_inout_port_glb_varname   = 4; // expecting VarName
static const int state_inout_port_glb_rbrkt     = 5; // expecting ']'
static const int state_inout_port_entry_end     = 6; // expecting ',' or ';'

static int state_current = state_inout_port_name;

static LexerToken_t port_name;
static size_t bitvec_size;

extern "C" bool parse_module_inout_port(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr,
        const int port_type) {

    //
    // valid in/out port declaration
    //
    // VarName;                 <-- implicitly single bit type
    // VarName[#constant];      <-- numeric constant used as bit vector size
    // VarName[VarName];        <-- this is not supported yet
    // VarName[global.VarName]; <-- can use global as size spec
    //
    // multiple port declarations can go on the same line:
    //
    // VarNameA, VarNameB[32], VarNameC[global.CpuWidth]; <-- always terminated with a semicolon (this is how the parser identifies the end)
    //

    //std::cout << "- parse_module_inout_port : " << lexer_token_name_and_value(token, src) << std::endl;

    switch(state_current) {
    case state_inout_port_name:
        if(token.type == LexerToken_VarName) {
            port_name = token;
            state_current = state_inout_port_name_followup;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting token of type [" + lexer_token_name(LexerToken_VarName) + 
                "]. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_name_followup:
        if(token.type == LexerToken_Syntax_Comma || token.type == LexerToken_Syntax_Semicolon) {
            handle_port_single_bit(port_type, port_name, src, module_ptr, filename);
            token_iter++;
            state_current = state_inout_port_name;
            return token.type == LexerToken_Syntax_Semicolon;
        }
        else if(token.type == LexerToken_Syntax_LBracket) {
            token_iter++;
            state_current = state_inout_port_index_1st;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting one of ',' or ';' or '['. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_index_1st: // can be #constant, VarName, or 'global'
        if(lexer_token_is_number_type(token)) {
            // numeric constant
            bitvec_size = lexer_token_number_to_ulong(token, src);
            state_current = state_inout_port_glb_rbrkt;
            token_iter++;
            return false;
        }
        else if(token.type == LexerToken_VarName) {
            throw_parse_error(
                "Using local variable as bit-vector size is not currently supported. "
                "Did you mean 'global." + lexer_token_value(token, src) + "'?",
                filename, src, token.start, token);
        }
        else if(token.type == LexerToken_KW_global) {
            // 'global'
            token_iter++;
            state_current = state_inout_port_glb_dot;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting 'global' or numeric type. Received token of type [" + 
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_glb_dot:
        if(token.type == LexerToken_Syntax_Period) {
            state_current = state_inout_port_glb_varname;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting '.'. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_glb_varname:
        if(token.type == LexerToken_VarName) {

            const std::string global_value_name = lexer_token_value(token, src);

            auto glb_iter = rt->globals.find(global_value_name);
            if(glb_iter == rt->globals.end()) {
                throw_parse_error(
                    "Global value '" + global_value_name + "' not found, "
                    "unable to evaluate bitvector size.",
                    filename, src, token.start, token);
            }
            else {
                const int wh = glb_iter->second.which();
                if(wh == hdl_variant_long) {
                    bitvec_size = boost::get<long>(glb_iter->second);
                }
                else if(wh == hdl_variant_ulong) {
                    bitvec_size = boost::get<size_t>(glb_iter->second);
                }
                else {
                    throw_parse_error(
                        "Global value '" + global_value_name + "' is not an integer type. Unable to evaluate bitvector size.",
                        filename, src, token.start, token);
                }
            }

            state_current = state_inout_port_glb_rbrkt;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting token of type [" + lexer_token_name(LexerToken_VarName) + 
                "]. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_glb_rbrkt: // ]
        if(token.type == LexerToken_Syntax_RBracket) {

            // add port to module
            handle_port_bitvector(
                port_type, bitvec_size, port_name, 
                src, module_ptr, filename);

            token_iter++;
            state_current = state_inout_port_entry_end;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting ']'. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_inout_port_entry_end: // ',' or ';'
        if(token.type == LexerToken_Syntax_Comma) {
            state_current = state_inout_port_name;
            token_iter++;
            return false;
        }
        else if(token.type == LexerToken_Syntax_Semicolon) {
            state_current = state_inout_port_name;
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

    }
}

static void dbg_print_ports(hdl_module_t* module_ptr, const std::vector<char>& src) {

    std::cout << "DBG PRINT ===============\n";

    for(auto& p : module_ptr->module_io_port_interface) {
        std::cout << "    '" << p.first << "' : ";

        if(p.second.which() == module_io_type_input) {
            module_input_t min = boost::get<module_input_t>(p.second);
            if(min.type == type_bit) {
                std::cout << "input,bit" << std::endl;
            }
            else {
                std::cout << "input,bitvector[size=" << min.bitvector.len << "]" << std::endl;
            }
        }
        else { // output
            module_output_t mout = boost::get<module_output_t>(p.second);
            if(mout.type == type_bit) {
                std::cout << "output,bit" << std::endl;
            }
            else {
                std::cout << "output,bitvector[size=" << mout.bitvector.len << "]" << std::endl;
            }
        }
    }

    std::cout << "DBG PRINT ===============\n";
}

static void handle_port_bitvector(
        const int port_type,
        const size_t port_size,
        const LexerToken_t& port_name, 
        const std::vector<char>& src, 
        hdl_module_t* module_ptr,
        const std::string& filename) {

    const auto src_iter = src.begin();
    std::string pname(src_iter + port_name.start, src_iter + port_name.end);

    if(port_type == module_io_type_input) {
        module_input_t min;
        min.type = type_bitvector;
        min.bitvector.len = port_size;

        auto port_iter = module_ptr->module_io_port_interface.find(pname);
        if(port_iter == module_ptr->module_io_port_interface.end()) {
            module_ptr->module_io_port_interface.insert({ pname, min });
        }
        else {
            // ERROR: port already exists in module
            throw_parse_error(
                "In/Out port with name '" + pname + "' already exists in module '" + module_ptr->name + "'.",
                filename, src, port_name.start, port_name);
        }
    }
    else {
        module_output_t mout;
        mout.type = type_bitvector;
        mout.bitvector.len = port_size;

        auto port_iter = module_ptr->module_io_port_interface.find(pname);
        if(port_iter == module_ptr->module_io_port_interface.end()) {
            module_ptr->module_io_port_interface.insert({ pname, mout });
        }
        else {
            // ERROR: port already exists in module
            throw_parse_error(
                "In/Out port with name '" + pname + "' already exists in module '" + module_ptr->name + "'.",
                filename, src, port_name.start, port_name);
        }
    }
}

static void handle_port_single_bit(
        const int port_type, 
        const LexerToken_t& port_name, 
        const std::vector<char>& src, 
        hdl_module_t* module_ptr,
        const std::string& filename) {

    const auto src_iter = src.begin();
    std::string pname(src_iter + port_name.start, src_iter + port_name.end);

    if(port_type == module_io_type_input) {
        module_input_t min;
        min.type = type_bit;

        auto port_iter = module_ptr->module_io_port_interface.find(pname);
        if(port_iter == module_ptr->module_io_port_interface.end()) {
            module_ptr->module_io_port_interface.insert({ pname, min });
        }
        else {
            // ERROR: port already exists in module
            dbg_print_ports(module_ptr, src);
            throw_parse_error(
                "In/Out port with name '" + pname + "' already exists in module '" + module_ptr->name + "'.",
                filename, src, port_name.start, port_name);
        }
    }
    else {
        module_output_t mout;
        mout.type = type_bit;

        auto port_iter = module_ptr->module_io_port_interface.find(pname);
        if(port_iter == module_ptr->module_io_port_interface.end()) {
            module_ptr->module_io_port_interface.insert({ pname, mout });
        }
        else {
            // ERROR: port already exists in module
            dbg_print_ports(module_ptr, src);
            throw_parse_error(
                "In/Out port with name '" + pname + "' already exists in module '" + module_ptr->name + "'.",
                filename, src, port_name.start, port_name);
        }
    }
}
