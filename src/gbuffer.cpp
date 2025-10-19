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

#include "gbuffer.hpp"

using namespace werk;

void gbuffer::accommodate(std::size_t extra)
{
	const auto rem_cap = gap_end - gap_begin;
	if (extra > rem_cap) {
		const auto old_cap = buf.size();
		buf.resize(buf.size() - rem_cap + extra);
		buf.resize(buf.capacity());
		auto shift = buf.size() - old_cap;
		std::copy_backward(buf.data() + gap_end, buf.data() + old_cap, buf.end());
		std::fill(buf.data() + gap_end, buf.data() + gap_end + shift, 0);
		gap_end += shift;
	}
}

std::span<char8_t> gbuffer::writable_gap()
{
	return {buf.data() + gap_begin, gap_end - gap_begin};
}

void gbuffer::insert(std::u8string_view sv)
{
	auto rem_cap = gap_end - gap_begin;
	accommodate(sv.size());
	std::copy(sv.begin(), sv.end(), writable_gap().begin());
	gap_begin += sv.size();
}

void gbuffer::erase_back(std::size_t n)
{
	n = std::min(n, gap_begin);
	gap_begin -= n;
	const auto it = writable_gap().begin();
	std::fill(it, it + n, 0);
}

void gbuffer::erase_forward(std::size_t n)
{
	n = std::min(n, buf.size() - gap_end);
	gap_end += n;
	const auto it = writable_gap().end();
	std::fill(it - n, it, 0);
}

void gbuffer::erase(iterator from, iterator to)
{
	auto g = gap();
	if (from <= g && g <= to) {
		auto back = g - from;
		auto fwd = to - g;
		erase_back(back);
		erase_forward(fwd);
	} else {
		// Minimise gap movement
		auto n = to - from;
		if (g < from) {
			place_gap(from);
			erase_forward(n);
		} else {
			place_gap(to);
			erase_back(n);
		}
	}
}

void gbuffer::place_gap(iterator pos)
{
	auto adv = pos - gap();
	if (adv == 0)
		return;

	auto old_gap_begin = buf.data() + gap_begin;
	auto old_gap_end = buf.data() + gap_end;
	auto new_gap_begin = buf.data() + gap_begin + adv;
	auto new_gap_end = buf.data() + gap_end + adv;
	if (adv > 0) {
		std::copy(old_gap_end, old_gap_end + adv, new_gap_begin);
		std::fill(std::max(old_gap_begin, new_gap_begin), new_gap_end, 0);
	} else {
		std::copy_backward(old_gap_begin + adv, old_gap_begin, new_gap_end - adv);
		std::fill(new_gap_begin, std::min(old_gap_end, new_gap_end), 0);
	}

	gap_begin += adv;
	gap_end += adv;
}
