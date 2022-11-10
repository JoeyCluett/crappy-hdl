#include "module-desc.h"
#include <src/runtime/runtime-env.h>
#include <src/error-util.h>

const bool module_desc_add_interface_element(
        module_desc_t* modptr,
        const std::string& el_name,
        module_desc_t::interface_type_t int_type,
        module_desc_t::interface_size_t int_size,
        size_t const_size) {

    return true;

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

size_t module_desc_get_idx_of_string(
        module_desc_t* modptr,
        const std::string& string_constant) {

    

}
