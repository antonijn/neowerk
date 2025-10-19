#include "gbuffer.hpp"

#include <algorithm>
#include <functional>

#include <gtest/gtest.h>

using namespace werk;

namespace {

TEST(GBufferTest, DefaultConstructor) {
	auto gbuf = gbuffer{};
	EXPECT_EQ(gbuf.begin(), gbuf.end());
}

TEST(GBufferTest, CopyConstructor) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"copy me");
	auto copy = gbuf;
	EXPECT_EQ(gbuf.size(), copy.size());
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.end(), copy.begin()));
}

TEST(GBufferTest, Initialization) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"hello, world");
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.end(), u8"hello, world"));
}

TEST(GBufferTest, PrependAppend) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"two,");
	gbuf.place_gap(gbuf.begin());
	gbuf.insert(u8"one,");
	gbuf.place_gap(gbuf.end());
	gbuf.insert(u8"three");
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.end(), u8"one,two,three"));
}

TEST(GBufferTest, EraseBack) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"erase this");
	gbuf.erase_back(5);
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.end(), u8"erase"));
}

TEST(GBufferTest, EraseForward) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"erase this, please");
	gbuf.place_gap(gbuf.end() - 13);
	gbuf.erase_forward(5);
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.end(), u8"erase, please"));
}

TEST(GBufferTest, OverEraseBack) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"XXXtest");
	gbuf.place_gap(gbuf.begin() + 3);
	gbuf.erase_back(10);
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.end(), u8"test"));
}

TEST(GBufferTest, OverEraseForward) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"testXXX");
	gbuf.place_gap(gbuf.end() - 3);
	gbuf.erase_forward(10);
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.end(), u8"test"));
}

TEST(GBufferTest, EraseRange) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"testXXXtest");
	gbuf.place_gap(gbuf.begin() + 5);
	gbuf.erase(gbuf.begin() + 4, gbuf.begin() + 7);
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.end(), u8"testtest"));
	gbuf.insert(u8"YYY");
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.end(), u8"testYYYtest"));
	gbuf.place_gap(gbuf.begin());
	gbuf.erase(gbuf.begin() + 4, gbuf.end());
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.end(), u8"testY"));
	EXPECT_EQ(gbuf.gap(), gbuf.end());
}

TEST(GBufferTest, GapIsZero) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"gap_mustXXXbe_zeroXXX");
	gbuf.erase_back(3);
	gbuf.place_gap(gbuf.gap() - 7);
	gbuf.erase_back(3);
	EXPECT_TRUE(std::equal(gbuf.begin(), gbuf.gap(), u8"gap_must"));
	EXPECT_TRUE(std::equal(gbuf.gap(), gbuf.end(), u8"be_zero"));
	auto data = gbuf.data();
	auto gap_ofs = gbuf.gap() - gbuf.begin();
	auto gap_size = gbuf.capacity() - (gbuf.end() - gbuf.begin());
	EXPECT_TRUE(std::all_of(data + gap_ofs, data + gap_ofs + gap_size, [](auto x){ return x == 0; }));
}

TEST(GBufferTest, Mutable) {
	auto gbuf = gbuffer{};
	gbuf.insert(u8"uppercase");
	std::for_each(gbuf.begin(), gbuf.end(), [](auto& x){ x = std::toupper(x); });
	EXPECT_TRUE(std::equal(gbuf.gap(), gbuf.end(), u8"UPPERCASE"));
}

}
