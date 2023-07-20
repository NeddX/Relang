#include <iostream>
#include <vector>

#include "Compiler.h"
#include "Lexer.h"

namespace std {
	std::vector<std::int8_t> to_bytes(std::string&& str) {
		return std::vector<std::int8_t>(str.begin(), str.end());
	}
	std::vector<std::int8_t> to_bytes(const std::string& str) {
		return std::vector<std::int8_t>(str.begin(), str.end());
	}
}

int main(int argc, const char* argv[])
{
	return 0;
}
