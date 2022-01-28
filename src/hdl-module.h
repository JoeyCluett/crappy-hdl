#pragma once

#include "hdl-variant.h"
#include <boost/variant.hpp>
#include <map>
#include <string>

struct module_input_t {
    int type;

    union {
        struct {
            size_t len;
        } bitvector;
    };

};

struct module_output_t {
    int type;

    union {
        struct {
            size_t len;
        } bitvector;
    };

};
const int type_bitvector = 0;
const int type_bit       = 1;

typedef boost::variant<module_input_t, module_output_t> module_io_t;
const int module_io_type_input  = 0;
const int module_io_type_output = 1;

struct hdl_module_t {
    int header_status;
    std::string name;
    std::vector<std::pair<std::string, hdl_variant_t>> argument_list;
    std::map<std::string, module_io_t> module_io_port_interface;
};

const int module_header_unknown   = 0;
const int module_header_empty     = 1;
const int module_header_not_empty = 2;

