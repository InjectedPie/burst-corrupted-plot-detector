/* Burst Corrupted Plot Detector - Scans Burst mining logs and reports 
 * possible corrupt plot files.
 * Copyright(C) 2018 Guney Ozsan
 *
 * This program is free software : you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>

namespace console_gui {
	std::string align_right(const std::string &content, const size_t &slot_size);
	std::string underline(const std::string &content);
	std::string move_cursor_back(const size_t &size);
	std::string whitespace(const size_t &size);
	std::string center(const std::string &content, const size_t &width);
};
