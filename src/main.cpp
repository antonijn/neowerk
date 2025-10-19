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
