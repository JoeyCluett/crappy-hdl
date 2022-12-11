#pragma once

#include <src/runtime/module-desc.h>
#include <iostream>

//
// print out disassembled source code
//
void disassemble_bytecode(
        std::ostream& os,
        struct module_desc_t* modptr);

