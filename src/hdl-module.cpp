#include "hdl-module.h"

#include <iostream>

static std::string rpad(const std::string& s, int len) {
    std::string s_ = s;
    while(s_.size() < len)
        s_.push_back(' ');
    return s_;
}

void module_arglist_print_info(hdl_module_t* m) {

    std::cout << "module '" << m->name << "'\n";

    const char* type_lut[] = {
        "long",
        "ulong",
        "string"
    };

    for(auto& p : m->argument_vector) {
        std::cout << "    " << rpad(p.first, 20) << " : " << type_lut[p.second] << std::endl;
    }
}

size_t module_arglist_add_argument_name(hdl_module_t* m, const std::string& argname, int* errorcode) {
    *errorcode = module_arg_error_no_error;

    auto arg_map_iter = m->argument_map.find(argname);

    if(arg_map_iter != m->argument_map.end()) {
        *errorcode = module_arg_error_argument_already_exists;
        return ~0ul;
    }

    // verified that argument doesnt already exist

    size_t arg_index = m->argument_vector.size();

    // place argument name in both structures:
    m->argument_vector.push_back({ argname, -1 });
    m->argument_map.insert({ argname, {-1, arg_index }});

    return arg_index;
}

const int module_arglist_set_argument_type(hdl_module_t* m, const size_t arg_index, const int arg_type) {

    if(arg_index >= m->argument_vector.size())
        return module_arg_error_invalid_arg_index;

    auto vec_entry = m->argument_vector[arg_index];
    auto arg_map_iter = m->argument_map.find(vec_entry.first);

    if(arg_map_iter == m->argument_map.end())
        return module_arg_error_name_not_found;

    // entry exists in arg vector and arg map
    m->argument_vector[arg_index].second = arg_type;
    arg_map_iter->second.first = arg_type;

    return module_arg_error_no_error;
}

std::string module_arglist_get_argument_name(hdl_module_t* m, const size_t arg_index) {
    return m->argument_vector.at(arg_index).first;
}

size_t module_arglist_get_argument_index(hdl_module_t* m, const std::string& str) {
    size_t sz;

    for(auto& p : m->argument_vector) {
        if(p.first == str) {
            return sz;
        }
        sz++;
    }

    return ~0ul;
}
