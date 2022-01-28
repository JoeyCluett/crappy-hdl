#pragma once

#include <boost/variant.hpp>

typedef boost::variant<long, size_t, std::string> hdl_variant_t;

const int hdl_variant_long   = 0;
const int hdl_variant_ulong  = 1;
const int hdl_variant_string = 2;

const std::string hdl_variant_str_repr(const hdl_variant_t& hdl_var);
