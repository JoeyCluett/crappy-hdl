#pragma once
// license info below

#include <src/runtime/module-desc.h>

#include <string>
#include <vector>

typedef std::vector<uint8_t> serialization_data_t;

void serialize_module_desc(serialization_data_t* const ser, module_desc_t* mod);

void serialize_save_to_file(serialization_data_t* const ser, const std::string& filename);

const bool serialize_util_file_exists(const std::string& filepath);

/*
Copyright (C) 2023  Joe Cluett aka SevenSignBits
This file is part of Crappy/Compass HDL.
Crappy/Compass HDL is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the 
Free Software Foundation, either version 3 of the License, or (at your option) any later version.
json-parser is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along 
with json-parser. If not, see <https://www.gnu.org/licenses/>.
*/
