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

#include <csignal>
#include <dlfcn.h>
#include <experimental/scope>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include <tree_sitter/api.h>

#include "gbuffer.hpp"

using namespace std::literals::string_view_literals;

static const char *read_from_gbuffer(void *payload, uint32_t byte_index, TSPoint position, uint32_t *bytes_read)
{
	auto& gbuffer = *static_cast<werk::gbuffer *>(payload);
	byte_index = std::min(byte_index, static_cast<uint32_t>(gbuffer.size()));
	auto it = gbuffer.begin() + byte_index;
	auto gap = gbuffer.gap();
	auto n = (it < gap) ? (gap - it) : (gbuffer.end() - it);
	*bytes_read = n;
	return (n > 0) ? reinterpret_cast<char *>(&*it) : nullptr;
}

auto open_file(char *path)
{
	using istreambuf_iterator = std::istreambuf_iterator<char>;

	auto file = std::ifstream{path, std::ios::binary};
	return werk::gbuffer{istreambuf_iterator{file}, istreambuf_iterator{}};
}

void print_node(const werk::gbuffer& gbuf, TSNode node)
{
	auto node_begin = gbuf.begin() + ts_node_start_byte(node);
	auto node_end = gbuf.begin() + ts_node_end_byte(node);

	auto eol = true;
	for (auto it = gbuf.begin(); ; ++it) {
		if (it == node_begin)
			std::cerr << "\x1B[1;6m";
		if (it == node_end)
			std::cerr << "\x1B[0m";
		if (it == gbuf.end())
			break;

		if (eol)
			std::cerr << "  ";

		auto c = *it;
		eol = c == u8'\n';
		std::cerr << (char)c;
	}

	if (!eol)
		std::cerr << std::endl;
}

static void reset_termbuf()
{
	std::cerr << "\x1B[?1049l";
}
static void alt_termbuf()
{
	std::cerr << "\x1B[?1049h";
}

static void handle_sigint(int signum)
{
	reset_termbuf();
	_exit(130);
}

int main(int argc, char **argv)
{
	using std::experimental::scope_exit;

	if (argc != 2) {
		std::cerr << "expect one file\n";
		return 1;
	}

	auto gbuf = open_file(argv[1]);

	auto ts = ts_parser_new();
	auto gts = scope_exit{[&]{ ts_parser_delete(ts); }};

	auto ploc = getenv("TS_PARSER");
	if (ploc == nullptr) {
		std::cerr << "TS_PARSER not set\n";
		return 1;
	}
	auto prs_so = dlopen(ploc, RTLD_LAZY);
	if (prs_so == nullptr) {
		std::cerr << "failed to open .so\n";
		return 1;
	}
	auto gprs_so = scope_exit{[&]{ dlclose(prs_so); }};

	auto lang_cb = (const TSLanguage *(*)())dlsym(prs_so, "tree_sitter_c");
	if (lang_cb == nullptr) {
		std::cerr << "failed to find tree_sitter_c\n";
		return 1;
	}
	auto lang = const_cast<TSLanguage *>(lang_cb());
	auto glang = scope_exit{[&]{ ts_language_delete(lang); }};

	if (!ts_parser_set_language(ts, lang)) {
		std::cerr << "error setting ts language\n";
		return 1;
	}

	auto input = TSInput{
		.payload = reinterpret_cast<void *>(&gbuf),
		.read = read_from_gbuffer,
		.encoding = TSInputEncodingUTF8,
		.decode = nullptr,
	};
	auto tree = ts_parser_parse(ts, nullptr, input);
	auto gtree = scope_exit{[&]{ ts_tree_delete(tree); }};

	const auto root = ts_tree_root_node(tree);
	auto node = root;
	auto repeat = std::string{};
	auto diag = std::stringstream{};

	auto commands = std::map<std::string_view, std::function<void()>>{};
	commands["lchild"sv] = [&]{ node = ts_node_child(node, 0); };
	commands["rchild"sv] = [&]{ node = ts_node_child(node, ts_node_child_count(node) - 1); };
	commands["lsib"sv] = [&]{ node = ts_node_prev_sibling(node); };
	commands["rsib"sv] = [&]{ node = ts_node_next_sibling(node); };
	commands["parent"sv] = [&]{ node = ts_node_parent(node); };
	commands["help"sv] = [&]{
		for (auto [name, _] : commands)
			diag << name << " ";
		diag << std::endl;
	};

	atexit(reset_termbuf);
	std::signal(SIGINT, handle_sigint);

	for (;;) {
		alt_termbuf();
		std::cerr << "\x1B[2J\x1B[H";
		print_node(gbuf, node);

		std::cerr << diag.str();
		diag.str("");
		diag.clear();

		if (ts_node_is_named(node)) {
			auto color = ts_node_is_extra(node) ? "35" : "34";
			std::cerr << "\x1B[" << color << "m" << ts_node_type(node);
		}
		std::cerr << "\x1B[0m> ";

		auto line = std::string{};
		if (!std::getline(std::cin, line))
			break;

		auto cmd = line.empty() ? repeat : line;

		auto prev_node = node;
		if (auto it = commands.find(cmd); it != commands.end())
			it->second();
		else
			diag << "\x1B[31munknown command:\x1B[0m " << cmd << "\n";

		if (ts_node_is_null(node)) {
			diag << "\x1B[31minvalid operation\x1B[0m\n";
			node = prev_node;
		}

		repeat = cmd;
	}

	std::cerr << std::endl;
	return 0;
}
