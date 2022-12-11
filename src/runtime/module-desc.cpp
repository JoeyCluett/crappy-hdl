#include "module-desc.h"
#include <src/runtime/runtime-env.h>
#include <src/error-util.h>
#include <src/semantic-analysis/parser.h>

#include <iostream>
#include <string>
#include <stdlib.h>

std::ostream& operator<<(std::ostream& os, const module_desc_t& modptr) {

    os << "module : " << modptr.name << "\n";
    os << "argument list:\n";
    for(auto& p : modptr.argument_list) {
        os << "    " << modptr.constants.at(p.first) << ", type[";
        switch(p.second) {
        case token_type_t::keyword_integer:  os << "integer]"; break;
        case token_type_t::keyword_uinteger: os << "uinteger]"; break;
        case token_type_t::keyword_string:   os << "string]"; break;
        default:
            os << "INVALID]"; break;
        }
        os << "\n";
    }

    os << "interface:\n";
    for(auto& p : modptr.interface_elements) {
        os << "    " << p.first << ", dir[";
        switch(std::get<0>(p.second)) {
        case module_desc_t::interface_type_t::in: os << "in], type["; break;
        case module_desc_t::interface_type_t::out: os << "out], type["; break;
        default: os << "INVALID], type["; break;
        }

        switch(std::get<1>(p.second)) {
        case module_desc_t::interface_size_t::single: os << "bit]"; break;
        case module_desc_t::interface_size_t::array: os << "array]"; break;
        default: os << "INVALID]"; break;
        }
        os << "\n";
    }

    return os;
}

std::tuple<bool, size_t, std::string> module_desc_add_interface_element(
        module_desc_t* modptr,
        const std::string& el_name,
        module_desc_t::interface_type_t int_type,
        module_desc_t::interface_size_t size_type) {

    size_t idx = module_desc_add_string_constant(modptr, el_name);
    auto iter = modptr->interface_elements.find(el_name);
    if(iter != modptr->interface_elements.end())
        return { false, 0ul, "Interface element with name `" + el_name + "' already exists in module `" + modptr->name + "'" };

    modptr->interface_elements.insert({ el_name, { int_type, size_type }});
    return { true, idx, "" };
}

size_t module_desc_add_string_constant(
        module_desc_t* modptr,
        const std::string& string_constant) {

    // TODO : use a map<> for the lookup
    size_t idx = 0ul;
    for(auto& s : modptr->constants) {
        if(s == string_constant) {
            return idx;
        }
        idx++;
    }

    // the constant does not exist in constant array
    idx = modptr->constants.size();
    modptr->constants.push_back(string_constant);
    return idx;
}

std::pair<bool, size_t> module_desc_get_idx_of_string(
        module_desc_t* modptr,
        const std::string& string_constant) {

    size_t idx = 0ul;
    for(auto& s : modptr->constants) {
        if(s == string_constant) {
            return { true, idx };
        }
        idx++;
    }

    return { false, 0ul }; // doesnt matter what second entry is
}

void module_desc_add_argument_desc(
        module_desc_t* modptr,
        parse_info_t& p,
        token_t& arg_name,
        token_t& arg_type) {

    const string_t arg_name_s = lexer_token_value(arg_name, p.src);

    for(auto& q : modptr->argument_list) {
        const string_t args = modptr->constants.at(q.first);
        if(args == arg_name_s)
            throw_parse_error(
                    "In module '" + modptr->name + "', argument with name '" + arg_name_s + "' already exists",\
                    p.filename, p.src, arg_name);
    }

    size_t arg_idx = module_desc_add_string_constant(modptr, arg_name_s);
    modptr->argument_list.push_back({ arg_idx, arg_type.type });
}
