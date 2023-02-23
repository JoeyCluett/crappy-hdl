#pragma once

#include <src/lexer.h>
#include <src/semantic-analysis/parser.h>

#include <stddef.h>

#include <string>
#include <map>
#include <tuple>
#include <vector>
#include <utility>
#include <iostream>

struct module_desc_t {
    std::string name;

    // used in the bytecode stream for various purposes. string references in bytecode
    // are referenced as indices into this table
    std::vector<std::string> constants;

    enum class interface_type_t {
        in, out
    };

    enum class interface_size_t {
        single,
        array  // evaluated at runtime
    };

    std::map<std::string, std::tuple<interface_type_t, interface_size_t> >
            interface_elements;

    std::vector<std::pair<size_t, token_type_t> >
            argument_list;

    std::vector<uint8_t> bytecode;

    long int scope_levels = 1;
};

std::ostream& operator<<(std::ostream& os, const module_desc_t& modptr);

//
// returns { <success>, <idx-of-name>, <error-message> }
//
std::tuple<bool, size_t, std::string> module_desc_add_interface_element(
        module_desc_t* modptr,
        const std::string& el_name,
        module_desc_t::interface_type_t int_type,
        module_desc_t::interface_size_t size_type);

size_t module_desc_add_string_constant(
        module_desc_t* modptr,
        const std::string& string_constant);

std::pair<bool, size_t> module_desc_get_idx_of_string(
        module_desc_t* modptr,
        const std::string& string_constant);

void module_desc_add_argument_desc(
        module_desc_t* modptr,
        struct parse_info_t& p,
        token_t& arg_name,
        token_t& arg_type);



