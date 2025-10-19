// The werk text editor
// Copyright (C) 2025  Antonie Blom
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>

#include "gbuffer.hpp"

using namespace std::literals::string_view_literals;

int main()
{
	auto gbuf = werk::gbuffer{};
	gbuf.insert(u8"hello, world!\n"sv);

	std::ranges::copy(gbuf, std::ostream_iterator<char>(std::cout));

	return 0;
}
