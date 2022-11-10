#pragma once

#include <src/lexer.h>
#include <src/semantic-analysis/parser.h>

#include <stddef.h>

#include <string>
#include <map>
#include <tuple>
#include <vector>
#include <utility>

struct module_desc_t {
    // redundant because the name is also stored in
    // the runtime env but i like it here as well
    std::string name;

    // used in the bytecode stream for various purposes. string references in bytecode
    // are referenced as indices into this table
    std::vector<std::string> constants;

    enum class interface_type_t {
        in, out
    };

    enum class interface_size_t {
        single,
        const_size, // constant size is encoded
        input_arg   // evaluated at runtime
    }; 

    std::map<std::string, std::tuple<interface_type_t, interface_size_t, size_t> >
            interface_elements;

    enum class argument_type_t {
        integer,
        uinteger,
        string
    };

    std::vector<std::pair<std::string, argument_type_t> >
            argument_list;

};

const bool module_desc_add_interface_element(
        module_desc_t* modptr,
        const std::string& el_name,
        module_desc_t::interface_type_t int_type,
        module_desc_t::interface_size_t int_size,
        size_t const_size = 0ul);

size_t module_desc_add_string_constant(
        module_desc_t* modptr,
        const std::string& string_constant);

size_t module_desc_get_idx_of_string(
        module_desc_t* modptr,
        const std::string& string_constant);
