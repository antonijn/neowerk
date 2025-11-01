#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include <range/v3/iterator/basic_iterator.hpp>

namespace werk
{

class gbuffer {
public:
	template<typename TG, typename TC>
	class basic_cursor {
	private:
		TG *gbuf;
		std::ptrdiff_t gap_rel_pos;
	public:
		using basic_gbuffer = std::remove_cv_t<TG>;

		friend class basic_cursor<const basic_gbuffer, const char8_t>;

		basic_cursor() = default;
		basic_cursor(TG& gbuf_, std::ptrdiff_t pos)
			: gbuf(&gbuf_), gap_rel_pos(pos)
		{
		}
		// Allow conversion from iterator -> const_iterator
		basic_cursor(const basic_cursor<basic_gbuffer, char8_t>& other)
			: gbuf(other.gbuf), gap_rel_pos(other.gap_rel_pos)
		{
		}

		TC& read() const
		{
			auto abs_pos = gap_rel_pos;
			abs_pos += (abs_pos >= 0) ? gbuf->gap_end : gbuf->gap_begin;
			return gbuf->buf[abs_pos];
		}

		bool equal(const basic_cursor& other) const
		{
			return gap_rel_pos == other.gap_rel_pos;
		}

		void next()
		{
			++gap_rel_pos;
		}

		void prev()
		{
			--gap_rel_pos;
		}

		void advance(std::ptrdiff_t dist)
		{
			gap_rel_pos += dist;
		}

		std::ptrdiff_t distance_to(const basic_cursor& other) const
		{
			return other.gap_rel_pos - gap_rel_pos;
		}
	};

	using cursor = basic_cursor<gbuffer, char8_t>;
	using iterator = ranges::basic_iterator<cursor>;
	using const_cursor = basic_cursor<const gbuffer, const char8_t>;
	using const_iterator = ranges::basic_iterator<const_cursor>;

	using value_type = char8_t;

private:
	std::vector<char8_t> buf{};
	std::size_t gap_begin{0}, gap_end{0};

	void accommodate(std::size_t extra);
	std::span<char8_t> writable_gap();

	auto iter_at(std::ptrdiff_t gap_rel_pos)
	{
		return iterator{cursor{*this, gap_rel_pos}};
	}
	auto iter_at(std::ptrdiff_t gap_rel_pos) const
	{
		return const_iterator{const_cursor{*this, gap_rel_pos}};
	}

public:
	constexpr gbuffer() noexcept = default;
	template<typename InputIt>
	gbuffer(InputIt first, InputIt last)
	{
		std::copy(first, last, std::inserter(*this, gap()));
	}
	gbuffer(const gbuffer&) = default;
	gbuffer(gbuffer&&) = default;

	size_t capacity() const
	{
		return buf.size();
	}
	size_t size() const
	{
		return buf.size() - (gap_end - gap_begin);
	}
	char8_t *data()
	{
		return buf.data();
	}
	const char8_t *data() const
	{
		return buf.data();
	}

	void insert(std::u8string_view sv);
	iterator insert(const_iterator iter, char8_t c);
	iterator erase_back(std::size_t n);
	iterator erase_forward(std::size_t n);
	iterator erase(const_iterator from, const_iterator to);
	void place_gap(const_iterator pos);
	iterator gap()
	{
		return iter_at(0);
	}
	const_iterator gap() const
	{
		return iter_at(0);
	}
	const_iterator cgap() const
	{
		return gap();
	}

	iterator begin()
	{
		return iter_at(-static_cast<std::ptrdiff_t>(gap_begin));
	}
	const_iterator begin() const
	{
		return iter_at(-static_cast<std::ptrdiff_t>(gap_begin));
	}
	const_iterator cbegin() const
	{
		return begin();
	}

	iterator end()
	{
		return iter_at(static_cast<std::ptrdiff_t>(buf.size() - gap_end));
	}
	const_iterator end() const
	{
		return iter_at(static_cast<std::ptrdiff_t>(buf.size() - gap_end));
	}
	const_iterator cend() const
	{
		return end();
	}

	char8_t& operator[](std::size_t n)
	{
		return *iter_at(n);
	}
	const char8_t& operator[](std::size_t n) const
	{
		return *iter_at(n);
	}

	gbuffer& operator=(const gbuffer&) = default;
	gbuffer& operator=(gbuffer&&) = default;
};

}
