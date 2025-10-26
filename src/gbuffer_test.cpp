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

#include <algorithm>
#include <functional>

#include <gtest/gtest.h>

using namespace std::literals::string_view_literals;
using namespace werk;

namespace {

TEST(GBufferTest, DefaultConstructor) {
	auto gbuf = gbuffer{};
	EXPECT_EQ(gbuf.begin(), gbuf.end());
}

TEST(GBufferTest, CopyConstructor) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"copy me"sv);
	auto copy = gbuf;
	EXPECT_EQ(gbuf.size(), copy.size());
	EXPECT_TRUE(std::ranges::equal(gbuf, copy));
}

TEST(GBufferTest, Initialization) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"hello, world"sv);
	EXPECT_TRUE(std::ranges::equal(gbuf, u8"hello, world"sv));
}

TEST(GBufferTest, PrependAppend) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"two,"sv);
	gbuf.place_gap(gbuf.begin());
	gbuf.insert(u8"one,"sv);
	gbuf.place_gap(gbuf.end());
	gbuf.insert(u8"three"sv);
	EXPECT_TRUE(std::ranges::equal(gbuf, u8"one,two,three"sv));
}

TEST(GBufferTest, EraseBack) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"erase this"sv);
	gbuf.erase_back(5);
	EXPECT_TRUE(std::ranges::equal(gbuf, u8"erase"sv));
}

TEST(GBufferTest, EraseForward) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"erase this, please"sv);
	gbuf.place_gap(gbuf.end() - 13);
	gbuf.erase_forward(5);
	EXPECT_TRUE(std::ranges::equal(gbuf, u8"erase, please"sv));
}

TEST(GBufferTest, OverEraseBack) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"XXXtest"sv);
	gbuf.place_gap(gbuf.begin() + 3);
	gbuf.erase_back(10);
	EXPECT_TRUE(std::ranges::equal(gbuf, u8"test"sv));
}

TEST(GBufferTest, OverEraseForward) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"testXXX"sv);
	gbuf.place_gap(gbuf.end() - 3);
	gbuf.erase_forward(10);
	EXPECT_TRUE(std::ranges::equal(gbuf, u8"test"sv));
}

TEST(GBufferTest, EraseRange) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"testXXXtest"sv);
	gbuf.place_gap(gbuf.begin() + 5);
	gbuf.erase(gbuf.begin() + 4, gbuf.begin() + 7);
	EXPECT_TRUE(std::ranges::equal(gbuf, u8"testtest"sv));
	gbuf.insert(u8"YYY"sv);
	EXPECT_TRUE(std::ranges::equal(gbuf, u8"testYYYtest"sv));
	gbuf.place_gap(gbuf.begin());
	gbuf.erase(gbuf.begin() + 5, gbuf.end());
	EXPECT_TRUE(std::ranges::equal(gbuf, u8"testY"sv));
	EXPECT_EQ(gbuf.gap(), gbuf.end());
}

TEST(GBufferTest, GapIsZero) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"gap_mustXXXbe_zeroXXX"sv);
	gbuf.erase_back(3);
	gbuf.place_gap(gbuf.gap() - 7);
	gbuf.erase_back(3);
	auto before_gap = std::ranges::subrange{gbuf.begin(), gbuf.gap()};
	EXPECT_TRUE(std::ranges::equal(before_gap, u8"gap_must"sv));
	auto after_gap = std::ranges::subrange{gbuf.gap(), gbuf.end()};
	EXPECT_TRUE(std::ranges::equal(after_gap, u8"be_zero"sv));
	auto data = gbuf.data();
	auto gap_ofs = gbuf.gap() - gbuf.begin();
	auto gap_size = gbuf.capacity() - (gbuf.end() - gbuf.begin());
	EXPECT_TRUE(std::all_of(data + gap_ofs, data + gap_ofs + gap_size, [](auto x){ return x == 0; }));
}

TEST(GBufferTest, Mutable) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"uppercase"sv);
	std::ranges::for_each(gbuf, [](auto& x){ x = std::toupper(x); });
	EXPECT_TRUE(std::ranges::equal(gbuf, u8"UPPERCASE"sv));
}

TEST(GBufferTest, InsertIter) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"held!"sv);

	auto ins = std::inserter(gbuf, gbuf.begin() + 2);
	std::ranges::copy(u8"llo, wor"sv, ins);

	EXPECT_TRUE(std::ranges::equal(gbuf, u8"hello, world!"sv));
}

}
