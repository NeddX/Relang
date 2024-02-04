#ifndef BLEND_BASM_UTILS_H
#define BLEND_BASM_UTILS_H

#include <sdafx.h>

namespace relang::rmc::utils {
	namespace string {
		inline std::vector<u8> ToBytes(std::string&& str)
		{
			return std::vector<u8>(str.begin(), str.end());
		}

		inline std::vector<u8> ToBytes(const std::string& str)
		{
			return std::vector<u8>(str.begin(), str.end());
		}

		inline std::string ToLowerCopy(const std::string& str)
		{
			std::string low_str = str;
			std::for_each(low_str.begin(), low_str.end(), [](char& c) { c = std::tolower(c); });
			return low_str;
		}
		inline std::string ToUpperCopy(const std::string& str)
		{
			std::string upper_str = str;
			std::for_each(upper_str.begin(), upper_str.end(), [](char& c) { c = std::toupper(c); });
			return upper_str;
		}
	}
}

#endif // BLEND_BASM_UTILS_H
