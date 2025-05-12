#pragma once

#include <string>

class StringHelper
{
public:

	static inline std::string Replace(const std::string& str, const std::string& from, const std::string& to) {
		if (from.empty()) return str;

		std::string result = str;
		size_t start_pos = 0;

		while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
			result.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}

		return result;
	}

	static inline std::vector<std::string> Split(const std::string& str, char delimiter) {
		std::vector<std::string> result;
		std::string current;

		for (char ch : str) {
			if (ch == delimiter) {
				result.push_back(current);
				current.clear();
			}
			else {
				current += ch;
			}
		}

		result.push_back(current); 
		return result;
	}
};
