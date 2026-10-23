#pragma once

// Base64 for putting binary data into text formats (JSON saves, the web save file system).

#include <cstdint>
#include <string>
#include <vector>

namespace Base64
{
	inline std::string Encode(const std::vector<uint8_t>& in)
	{
		static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string out;
		out.reserve((in.size() + 2) / 3 * 4);
		uint32_t val = 0;
		int valb = -6;
		for (uint8_t c : in)
		{
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0)
			{
				out.push_back(chars[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6)
			out.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
		while (out.size() % 4)
			out.push_back('=');
		return out;
	}

	// Stops at the first character that is not part of the alphabet (the padding, or garbage).
	inline std::vector<uint8_t> Decode(const std::string& in)
	{
		static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::vector<uint8_t> out;
		std::vector<int> table(256, -1);
		for (int i = 0; i < 64; i++)
			table[static_cast<uint8_t>(chars[i])] = i;
		uint32_t val = 0;
		int valb = -8;
		for (char c : in)
		{
			const int index = table[static_cast<uint8_t>(c)];
			if (index == -1)
				break;
			val = (val << 6) + static_cast<uint32_t>(index);
			valb += 6;
			if (valb >= 0)
			{
				out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
		return out;
	}
}
