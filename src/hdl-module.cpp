#include "hdl-module.h"

#include <iostream>
#include <vector>
#include <assert.h>

/*
struct hdl_module_t {
    int header_status;
    std::string name;

    // waste of memory, but each argument has entries in two different structures
    std::vector<std::pair<std::string, int>> argument_vector;   // { name, type }  type is one of hdl_variant_*
    std::map<std::string, std::pair<int, size_t>> argument_map; // { nane, { type, index (in arg vector) } } type is one of hdl_variant_*

    std::map<std::string, module_port_t> module_io_port_interface;
    size_t n_in_ports;
    size_t n_out_ports;

    std::vector<std::string> string_array;
    std::vector<uint8_t> byte_code;
};
*/

void module_serialize(hdl_module_t* m, const std::vector<char>& src, std::ostream& os) {

    os << "\n\nNAME " << m->name << "\n";

    assert(m->argument_vector.size() == m->argument_map.size());

    os << "ARGSIZE " << m->argument_vector.size() << "\n";
    for(size_t i = 0ul; i < m->argument_vector.size(); i++) {
        const auto& p = m->argument_vector.at(i);
        os << "ARG " << p.first << ' ' << "TYPE:" << p.second << "\n";
    }

    os << "PORTSIZE " << m->module_io_port_interface.size() << "\n";
    for(auto& p : m->module_io_port_interface) {
        os << "PORT " << p.first << " IO:" << p.second.io_type << " TYPE:" << p.second.type << " EXT:" << p.second.global_index << "\n";
    }

    os << "STRINGSIZE " << m->string_array.size() << "\n";
    for(auto& s : m->string_array) {
        os << "STR " << s << "\n";
    }

    os << "BYTECODESIZE " << m->byte_code.size() << "\n";
    for(auto b : m->byte_code) {
        os << "0123456789ABCDEF"[(b >> 4) & 0x0F];
        os << "0123456789ABCDEF"[(b >> 0) & 0x0F];
        os << ' ';
    }

    os << "SRC\n";
    auto src_iter = src.begin();
    auto end_iter = src.end();

    while(src_iter != end_iter) {
        for(int i = 0; i < 32; i++) {
            if(src_iter == end_iter) {
                break;
            }

            const char b = *src_iter;
            os << "0123456789ABCDEF"[(b >> 4) & 0x0F];
            os << "0123456789ABCDEF"[(b >> 0) & 0x0F];

            src_iter++;
        }
        os << "\n";
    }


    os << std::endl;
}

size_t module_byte(hdl_module_t* m, const uint8_t bytecode) {
    m->byte_code.push_back(bytecode);
    return m->byte_code.size();
}

size_t module_u32(hdl_module_t* m, const uint32_t u32) {
    union {
        uint8_t buf[4];
        uint32_t u32;
    } u;

    u.u32 = u32;

    for(int i = 0; i < 4; i++)
        m->byte_code.push_back(u.buf[i]);

    return m->byte_code.size();
}

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
