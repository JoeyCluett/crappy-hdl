#pragma once

#include <iostream>
#include <map>
#include <string>

#include "lexical-token.h"
#include "hdl-variant.h"
#include "hdl-module.h"

struct HDL_Runtime_t {

    // <filename, 0/1=not imported/imported>
    std::map<std::string, int> import_names;

    std::map<std::string, hdl_variant_t> globals;

    std::map<std::string, hdl_module_t*> modules;

    std::vector<std::string> string_constants;
};

void hdl_runtime_add_import_name(HDL_Runtime_t* rt, const std::string& file_to_import);

bool hdl_runtime_has_unimported_files(HDL_Runtime_t* rt);

//
// returns globally unique index for this constant
//
int hdl_runtime_allocate_string_constant(const std::string& constant);

//
// try to add global variable
//
const int HDL_ADD_GLOBAL_SUCCESS        = 0;
const int HDL_ADD_GLOBAL_ALREADY_EXISTS = 1;
const int HDL_ADD_GLOBAL_INVALID_TYPE   = 2;
const int hdl_runtime_add_global_value(HDL_Runtime_t* rt, const std::string& var_name, LexerToken_t value, const std::vector<char>& src);

//
// print out globals
//
void hdl_runtime_print(std::ostream& os, HDL_Runtime_t* rt);

//
// add module (pointer) to runtime
// throws exception on error
//
const int HDL_ADD_MODULE_SUCCESS        = 0;
const int HDL_ADD_MODULE_ALREADY_EXISTS = 1;
const int hdl_runtime_add_module(HDL_Runtime_t* rt, hdl_module_t* module);
