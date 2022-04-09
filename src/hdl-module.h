#pragma once

#include "hdl-variant.h"
#include <boost/variant.hpp>
#include <map>
#include <string>
#include <vector>

struct module_port_t {
    int io_type;
    int type;

    //
    // depending on type, this can be a global or argument name index
    // field is ignored for single bit types
    //
    union {
        size_t argument_index; // which entry in the argument list should be used
        size_t global_index;   // which entry in global variables table should be used
        size_t vector_size;    // used when a constant is used to describe the size of the vector
    };
};



//
// for module_port_t::io_type
//
const int module_port_input  = 0;
const int module_port_output = 1;

//
// for module_port_t::type
//
const int module_port_type_bit              = 0;
const int module_port_type_bitvector_const  = 1; // size is numeric constant
const int module_port_type_bitvector_local  = 2; // size is module argument (index into hdl_module_t::argument_list)
const int module_port_type_bitvector_global = 3; // size is global constant (index into hdl_module_t::string_array)

//
// used when parsing module header
//
const int module_header_unknown   = 0;
const int module_header_empty     = 1;
const int module_header_not_empty = 2;

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
    std::vector<unsigned int> byte_code;
};

const int module_arg_error_no_error                = 0;
const int module_arg_error_argument_already_exists = 1;
const int module_arg_error_invalid_arg_index       = 2;
const int module_arg_error_name_not_found          = 3;

//
// print contents (names and types) of argument list
//
void module_arglist_print_info(hdl_module_t* m);

//
// add new argument to HDL module
// returns index to new argument
//
size_t module_arglist_add_argument_name(hdl_module_t* m, const std::string& argname, int* errorcode);

//
// set type for argument in HDL module
// returns error code
//
const int module_arglist_set_argument_type(hdl_module_t* m, const size_t arg_index, const int arg_type);

//
// get argument name from index (returned by other function)
// no error checking
//
std::string module_arglist_get_argument_name(hdl_module_t* m, const size_t arg_index);

//
// get argument index from name
//
size_t module_arglist_get_argument_index(hdl_module_t* m, const std::string& str);
