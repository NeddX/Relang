#ifndef ALVM_ALA_UTILS_H
#define ALVM_ALA_UTILS_H

#include <sdafx.h>

namespace rlang::rmc::utils {
	namespace string {
		inline std::vector<std::uint8_t> ToBytes(std::string&& str)
		{
			return std::vector<std::uint8_t>(str.begin(), str.end());
		}

		inline std::vector<std::uint8_t> ToBytes(const std::string& str)
		{
			return std::vector<std::uint8_t>(str.begin(), str.end());
		}

		inline std::string ToLowerCopy(const std::string& str)
		{
			std::string low_str = str;
			std::for_each(low_str.begin(), low_str.end(), [](char& c) { c = std::tolower(c); });
			return low_str;
		}
	}
}

#endif // ALVM_ALA_UTILS_H
