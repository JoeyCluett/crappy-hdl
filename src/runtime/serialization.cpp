#pragma once
// license info below

#include <src/runtime/serialization.h>

// store in little-endian format
static void serialize_ulong(serialization_data_t* const ser, size_t u64) {
    if(u64 == 0ul) {
        ser->push_back(0x00);
        return;
    }

    while(u64) {
        if(u64 > 127) {
            // MSB = 1
            ser->push_back(0x80 | (u64 & 0x7F));
        } else {
            // MSB = 0
            ser->push_back(u64 & 0x7F);
        }
        u64 >>= 7;
    }
}

static void serialize_string(serialization_data_t* const ser, const std::string& s) {

    static_assert(sizeof(size_t) == 8ul);

    size_t idx = 0ul;
    uint8_t byte = 0ul;

    // store ASCII bytes in packed format. uses 7 bytes to store 8 chars
    // is this amount of work worth (at most) a 12.5% reduction in size?
    // no.
    // will i do it anyway?
    // yes.
    for(const uint8_t u8 : s) {

        if(idx) {
            byte |= (u8 >> (7ul - idx));
            ser->push_back(byte);
        }
        byte = (u8 << (idx + 1));

        idx++;
        idx &= 0x07;
    }

    if(idx == 1ul)
        ser->push_back(byte);

    return;
}

void serialize_module_desc(serialization_data_t* const ser, module_desc_t* mod) {
    ser->clear();

    // serialize strings
    serialize_ulong(ser, mod->constants.size());
    for(const std::string& s : mod->constants) {
        serialize_ulong(ser, s.size());
        serialize_string(ser, s);
    }
}

void serialize_save_to_file(serialization_data_t* const ser, const std::string& filename);

const bool serial_util_file_exists(const std::string& filepath);

/*
Copyright (C) 2023  Joe Cluett aka SevenSignBits
This file is part of Crappy/Compass HDL.
Crappy HDL is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the 
Free Software Foundation, either version 3 of the License, or (at your option) any later version.
Crappy HDL is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along 
with Crappy HDL. If not, see <https://www.gnu.org/licenses/>.
*/