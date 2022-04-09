#include "hdl-runtime.h"

#include <iostream>
#include <string>

#include <src/error-util.h>

void hdl_runtime_toplevel_delete(HDL_Runtime_t* rt) {
    for(auto& p : rt->modules) {
        delete p.second;
    }

    delete rt;
}

void hdl_runtime_add_import_name(
        HDL_Runtime_t* rt, 
        const std::string& file_to_import) {

    auto iter = rt->import_names.find(file_to_import);
    if(iter == rt->import_names.end()) {
        // add new entry
        // import will be evaluated after current file is processed
        rt->import_names.insert({ file_to_import, 0 });
    }

}

bool hdl_runtime_has_unimported_files(HDL_Runtime_t* rt) {

    for(auto& p : rt->import_names) {
        if(p.second == 0)
            return true;
    }

    return false;
}

std::string hdl_runtime_next_unimported_file(HDL_Runtime_t* rt) {
    for(auto& p : rt->import_names) {
        if(p.second == 0)
            return p.first;
    }
    
    // returns empty string if nothing left to import
    return "";
}

int hdl_runtime_allocate_string_constant(HDL_Runtime_t* rt, const std::string& constant) {
    int next_idx = (int)rt->string_constants.size();
    rt->string_constants.push_back(constant);
    return next_idx;
}

const int hdl_runtime_add_global_value(HDL_Runtime_t* rt, const std::string& var_name, LexerToken_t value, const std::vector<char>& src) {
    auto iter = rt->globals.find(var_name);

    if(iter == rt->globals.end()) {
        if(value.type == LexerToken_NumberBin || value.type == LexerToken_NumberHex) {
            size_t ulong = lexer_token_number_to_ulong(value, src);
            rt->globals.insert({var_name, ulong});
            return HDL_ADD_GLOBAL_SUCCESS;
        }
        else if(value.type == LexerToken_NumberDec) {
            long int l = lexer_token_number_to_long(value, src);
            rt->globals.insert({var_name, l});
            return HDL_ADD_GLOBAL_SUCCESS;
        }
        else if(value.type == LexerToken_StringLiteral) {
            std::string token_str = lexer_token_string_eval(value, src);
            rt->globals.insert({ var_name, token_str });
            return HDL_ADD_GLOBAL_SUCCESS;
        }
        else {
            return HDL_ADD_GLOBAL_INVALID_TYPE;
        }
    }

    return HDL_ADD_GLOBAL_ALREADY_EXISTS;
}

void hdl_runtime_print(std::ostream& os, HDL_Runtime_t* rt) {

    os << "Globals:\n";
    for(auto& p : rt->globals) {
        os << "    " << p.first << " = " << hdl_variant_str_repr(p.second) << std::endl;
    }

    os << "\nModules:\n";
    for(auto& p : rt->modules) {
        os << "  " << p.first << "\n";
        for(auto& ports : p.second->module_io_port_interface) {

            //os << "    " << ports.first << " : \n";

        }
    }

    os << std::endl;
}

const int hdl_runtime_add_module(HDL_Runtime_t* rt, hdl_module_t* module) {
    auto module_iter = rt->modules.find(module->name);
    if(module_iter == rt->modules.end()) {
        // add the module
        rt->modules.insert({ module->name, module });
        return HDL_ADD_MODULE_SUCCESS;
    }
    else {
        return HDL_ADD_MODULE_ALREADY_EXISTS;
    }
}



