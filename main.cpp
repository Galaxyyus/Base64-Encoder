#include <iostream>
#include <ios>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdint>
#include <vector>
#include <unordered_map>

const char BASE64_ENCODE[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z',

	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
	'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
	'y', 'z',

	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9',

	'+', '/'
};

const std::unordered_map<char, int> BASE64_DECODE = {
	{'A',  0}, {'B',  1}, {'C',  2}, {'D',  3},
	{'E',  4}, {'F',  5}, {'G',  6}, {'H',  7},
	{'I',  8}, {'J',  9}, {'K', 10}, {'L', 11},
	{'M', 12}, {'N', 13}, {'O', 14}, {'P', 15},
	{'Q', 16}, {'R', 17}, {'S', 18}, {'T', 19},
	{'U', 20}, {'V', 21}, {'W', 22}, {'X', 23},
	{'Y', 24}, {'Z', 25},

	{'a', 26}, {'b', 27}, {'c', 28}, {'d', 29},
	{'e', 30}, {'f', 31}, {'g', 32}, {'h', 33},
	{'i', 34}, {'j', 35}, {'k', 36}, {'l', 37},
	{'m', 38}, {'n', 39}, {'o', 40}, {'p', 41},
	{'q', 42}, {'r', 43}, {'s', 44}, {'t', 45},
	{'u', 46}, {'v', 47}, {'w', 48}, {'x', 49},
	{'y', 50}, {'z', 51},

	{'0', 52}, {'1', 53}, {'2', 54}, {'3', 55},
	{'4', 56}, {'5', 57}, {'6', 58}, {'7', 59},
	{'8', 60}, {'9', 61},

	{'+', 62},
	{'/', 63},
	{'=', 0}
};

std::string base64_encode(const std::vector<char>& input) {
	std::stringstream result;

	std::size_t i = 0;
	while (i < input.size()) {
		std::uint32_t a = static_cast<std::uint8_t>(input[i++]);
		std::uint32_t b = (i < input.size()) ? static_cast<std::uint8_t>(input[i++]) : 0;
		std::uint32_t c = (i < input.size()) ? static_cast<std::uint8_t>(input[i++]) : 0;

		std::uint32_t bytes = (a << 16) | (b << 8) | c;

		for (int i = 3; i >= 0; i--) {
			std::uint8_t bitset = (bytes >> (6 * i)) & 0b00111111;
			result << BASE64_ENCODE[bitset];
		}
	}

	std::string b64_string = result.str();

	if (input.size() % 3 == 1) {
		b64_string = b64_string.substr(0, b64_string.size() - 2) + "==";
	} else if (input.size() % 3 == 2) {
		b64_string = b64_string.substr(0, b64_string.size() - 1) + "=";
	}

	return b64_string;
}

std::vector<char> base64_decode(const std::string& b64_string) {
	std::vector<char> result;

	for (std::size_t i = 0; i < b64_string.size(); i += 4) {
		std::uint32_t bytes = 0;
		for (std::size_t j = 0; j < 4; j++) {
			bytes = (bytes << 6) | (BASE64_DECODE.at(b64_string[i + j]) & 0b00111111);
		}

		for (int j = 2; j >= 0; j--) {
			char bitset = (bytes >> (8 * j)) & 0b11111111;
			result.push_back(bitset);
		}
	}

	if (b64_string.substr(b64_string.size() - 2) == "==") {
		result.pop_back();
		result.pop_back();
	} else if (b64_string.substr(b64_string.size() - 1) == "=") {
		result.pop_back();
	}

	return result;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cout << "Invalid Input!!!\n\n";
		std::cout << "Usage: b64.exe <OPTIONS> <STRING>\n\n";
		std::cout << "Options:\n";
		std::cout << "\t-e, --encode\tEncode a string to its Base64 equivalent\n";
		std::cout << "\t-d, --decode\tDecode a Base64 string to its ASCII equivalent\n";
		std::cout << "\nInput: Only valid C++ readable strings accepted. Input them as ASCII or Base64 as per requirement";
		return 0;
	}

	std::string option = argv[1];
	if (option == "-e" || option == "--encode") {
		std::ifstream file(argv[2], std::ios::binary | std::ios::ate);
		std::fstream::pos_type file_length = file.tellg();
		std::vector<char> input(file_length);
		file.seekg(0);
		file.read(input.data(), file_length);

		std::string b64_string = base64_encode(input);

		std::ofstream("base64.txt", std::ios::trunc) << b64_string;
	} else if (option == "-d" || option == "--decode") {
		std::string b64_string;
		std::ifstream(argv[2]) >> b64_string;

		std::vector<char> output_data = base64_decode(b64_string);

		std::ofstream("converted.png", std::ios::binary).write(output_data.data(), output_data.size());
	} else {
		std::cout << "Unknown option: " << option << '\n';
		return 1;
	}

	return 0;
}