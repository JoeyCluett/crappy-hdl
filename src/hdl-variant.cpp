#include "hdl-variant.h"

#include <sstream>

const std::string hdl_variant_str_repr(const hdl_variant_t& hdl_var) {

    std::stringstream ss;

    switch(hdl_var.which()) {
    case hdl_variant_long  :
        ss << boost::get<long>(hdl_var);
        break;
    case hdl_variant_ulong :
        ss << boost::get<size_t>(hdl_var);
        break;
    case hdl_variant_string:
        return "\"" + boost::get<std::string>(hdl_var) + "\"";
    }

    return ss.str();
}
