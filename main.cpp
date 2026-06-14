#include <iostream>
#include <ios>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <optional>

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
	{'/', 63}
};

// Converts a vector of bytes to its corresponding base64 string
// Returns std::nullopt if invalid string
std::string base64_encode(const std::vector<std::uint8_t>& input) {
	std::stringstream result;

	// Extracts 3 bytes from input if available then convert to 4 base64 characters
	std::size_t i = 0;
	while (i < input.size()) {
		std::uint32_t a = input[i++];
		std::uint32_t b = (i < input.size()) ? input[i++] : 0;
		std::uint32_t c = (i < input.size()) ? input[i++] : 0;

		std::uint32_t bytes = (a << 16) | (b << 8) | c;

		for (int j = 3; j >= 0; j--) {
			std::uint8_t bitset = (bytes >> (6 * j)) & 0b00111111; // Bit map to take least significant 6 bits
			result << BASE64_ENCODE[bitset];
		}
	}

	std::string b64_string = result.str();

	// Remove the last characters and add = depending on input length
	if (input.size() % 3 == 1) {
		b64_string = b64_string.substr(0, b64_string.size() - 2) + "==";
	} else if (input.size() % 3 == 2) {
		b64_string = b64_string.substr(0, b64_string.size() - 1) + "=";
	}

	return b64_string;
}

// Convert a base64 string to its corresponding bytes and returns a vector
std::optional<std::vector<std::uint8_t>> base64_decode(const std::string& b64_string) {
	if (b64_string.size() % 4 != 0) return std::nullopt;

	std::vector<uint8_t> result;

	// Count padding before processing
	int padding = 0;
	if (b64_string.size() >= 1 && b64_string.back() == '=') padding++;
	if (b64_string.size() >= 2 && b64_string[b64_string.size() - 2] == '=') padding++;

	// Read 4 characters from the base64 string and convert to 3 corresponding decoded bytes
	for (std::size_t i = 0; i < b64_string.size(); i += 4) {
		// Read 4 6 bit bytes
		std::uint32_t bytes = 0;
		for (std::size_t j = 0; j < 4; j++) {
			if (i + j < b64_string.size() - padding) {
				auto it = BASE64_DECODE.find(b64_string[i + j]);
				if (it == BASE64_DECODE.end()) return std::nullopt; // If invalid character found then return std::nullopt
				bytes = (bytes << 6) | (it->second & 0b00111111);
			} else {
				bytes <<= 6;
			}
		}

		// Extract the real decoded bytes
		for (int j = 2; j >= 0; j--) {
			uint8_t bitset = (bytes >> (8 * j)) & 0b11111111;
			result.push_back(bitset);
		}
	}

	// Remove the last bytes depending on padding number
	for (int i = 0; i < padding; i++) result.pop_back();

	return result;
}

int main(int argc, char* argv[]) {
	// Help for invalid inputs
	if (argc < 3) {
		std::cout << "Invalid Input!!!\n\n";
		std::cout << "Usage: b64.exe <OPTIONS> <INPUT> [FLAGS...]\n\n";
		std::cout << "Options:\n";
		std::cout << "\t-e, --encode\tEncode a string to its Base64 equivalent\n";
		std::cout << "\t-d, --decode\tDecode a Base64 string to its ASCII equivalent\n";
		std::cout << "\nFlags:\n";
		std::cout << "\t-f, --file <path>\tRead input from a file instead of a command-line string\n";
		std::cout << "\t-o, --output <path>\tWrite output to a file instead of stdout\n";
		std::cout << "\nExamples:\n";
		std::cout << "\tb64.exe -e \"Hello World\"\n";
		std::cout << "\tb64.exe -e -f input.png -o output.txt\n";
		std::cout << "\tb64.exe -d SGVsbG8gV29ybGQ=\n -o output.txt";
		std::cout << "\tb64.exe -d -f encoded.txt\n";
		return 0;
	}

	// Check if there is an -e or -d
	std::string option = argv[1];
	if (option != "-e" && option != "--encode" && option != "-d" && option != "--decode") {
		std::cout << "Unknown option: " << option << '\n';
		return 1;
	}

	bool is_encode = (option == "-e" || option == "--encode");

	std::string input_file, output_file, inline_input;

	// Parse remaining flags
	for (int i = 2; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "-f" || arg == "--file") {
			if (i + 1 >= argc) {
				std::cout << "Error: -f requires a file path argument\n";
				return 1;
			}
			input_file = argv[++i];
		} else if (arg == "-o" || arg == "--output") {
			if (i + 1 >= argc) {
				std::cout << "Error: -o requires a file path argument\n";
				return 1;
			}
			output_file = argv[++i];
		} else if (arg[0] != '-') {
			if (!inline_input.empty()) {
				std::cout << "Error: multiple inline inputs provided\n";
				return 1;
			}
			inline_input = arg;
		} else {
			std::cout << "Unknown flag: " << arg << '\n';
			return 1;
		}
	}

	// Both and neither input file and inline input provided
	if (!input_file.empty() && !inline_input.empty()) {
		std::cout << "Error: cannot use both -f and an inline input string\n";
		return 1;
	}
	if (input_file.empty() && inline_input.empty()) {
		std::cout << "Error: no input provided. Use -f <path> or pass a string directly\n";
		return 1;
	}

	// Handle encode operations
	if (is_encode) {
		std::vector<std::uint8_t> input;

		// Read from input_file
		if (!input_file.empty()) {
			std::ifstream file(input_file, std::ios::binary | std::ios::ate);
			if (!file) { // Could not open file
				std::cout << "Error: could not open file: " << input_file << '\n';
				return 1;
			}

			// Create a vector and read from file
			std::fstream::pos_type file_length = file.tellg();
			input.resize(file_length);
			file.seekg(0);
			file.read(reinterpret_cast<char*>(input.data()), file_length);
		}
		// Read from inline_input
		else {
			input.assign(inline_input.begin(), inline_input.end());
		}

		std::string b64_string = base64_encode(input);

		// Write to output_file
		if (!output_file.empty()) {
			std::ofstream out(output_file, std::ios::trunc);
			if (!out) {
				std::cout << "Error: could not open output file: " << output_file << '\n';
				return 1;
			}
			out << b64_string;
		}
		// Write to stdout
		else {
			std::cout << b64_string << '\n';
		}

	}
	// Handle decode operations
	else {
		std::string b64_string = "";

		// Read from input_file
		if (!input_file.empty()) {
			std::ifstream file(input_file);
			if (!file) {
				std::cout << "Error: could not open file: " << input_file << '\n';
				return 1;
			}
			std::string buffer;
			while (file >> buffer) {
				b64_string += buffer;
			}
		}
		// Read from inline_input
		else {
			b64_string = inline_input;
		}

		auto output_data = base64_decode(b64_string);
		if (!output_data) {
			std::cout << "Error invalid Base64 input!!! Aborting operation!";
			return 1;
		}

		// Write to output_file
		if (!output_file.empty()) {
			std::ofstream out(output_file, std::ios::binary);
			if (!out) {
				std::cout << "Error: could not open output file: " << output_file << '\n';
				return 1;
			}
			out.write(reinterpret_cast<char*>(output_data->data()), output_data->size());
		}
		// Write to stdout
		else {
			std::cout.write(reinterpret_cast<char*>(output_data->data()), output_data->size());
			std::cout << '\n';
		}
	}

	return 0;
}