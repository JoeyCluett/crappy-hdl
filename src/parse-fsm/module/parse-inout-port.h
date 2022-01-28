#pragma once

#include <vector>
#include <string>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>

const int PORT_LIST_TYPE_IN  = 0;
const int PORT_LIST_TYPE_OUT = 1;

extern "C" bool parse_module_inout_port(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr,
        const int port_type);

