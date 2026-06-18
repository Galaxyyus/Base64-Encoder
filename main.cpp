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

// This follows the modern RFC 4648 format which does not include line breaks in the output
bool base64_encode(std::istream& in, std::ostream& out) {
	const std::size_t CHUNK_SIZE = 3 * 1024; // Choosing 3kb to keep it a multiple of 3
	std::vector<std::uint8_t> buffer(CHUNK_SIZE);
	// Extra array to carry the leftover bits in case of incomplete reads
	std::uint8_t leftover[2];
	std::size_t leftover_count = 0;

	std::vector<char> out_buffer;
	out_buffer.reserve(CHUNK_SIZE * 4 / 3 + 8);

	while (true) {
		// Copy the leftover bytes from last operation into the buffer first
		for (int i = 0; i < leftover_count; i++) buffer[i] = leftover[i];

		// Read the next chunk of bytes
		in.read(reinterpret_cast<char*>(buffer.data() + leftover_count), CHUNK_SIZE - leftover_count);

		// Error checks
		if (in.bad()) {
			std::cerr << "Error: input stream encountered an I/O error!\n";
			return false;
		}
		if (in.fail() && !in.eof()) {
			std::cerr << "Error: input stream encounter an unknown error!\n";
			return false;
		}

		auto bytes_read = in.gcount();
		if (bytes_read == 0) break; // Reached EOF
		auto total_bytes = bytes_read + leftover_count;
		leftover_count = 0;

		// Process the current buffer
		// If the end of buffer is reached without 3 whole bytes to process then add those bytes to the
		// leftover buffer to be processed in the next iteration or at the end
		std::size_t i = 0;
		std::vector<char> out_buffer;
		out_buffer.reserve(CHUNK_SIZE * 4 / 3 + 8);
		while (i + 2 < total_bytes) {
			std::uint8_t a, b, c;
			a = buffer[i++];
			b = buffer[i++];
			c = buffer[i++];

			std::uint32_t bytes = (a << 16) | (b << 8) | c;

			for (int j = 3; j >= 0; j--) {
				// Only take the least significant 6 bits out of the 8 bits for encoding
				std::uint8_t bitset = (bytes >> (6 * j)) & 0b00111111;
				out_buffer.push_back(BASE64_ENCODE[bitset]);
			}
		}

		while (i < total_bytes) {
			leftover[leftover_count++] = buffer[i++];
		}

		out.write(out_buffer.data(), out_buffer.size());
		out.clear();
	}

	// If some bytes were not process at the end of the encoding
	if (leftover_count > 0) {
		// Process those bytes
		std::uint32_t bytes = (leftover[0] << 16) | (leftover[1] << 8);
		if (leftover_count == 1) {
			out.put(BASE64_ENCODE[(bytes >> (18)) & 0b00111111]);
			out.put(BASE64_ENCODE[(bytes >> (12)) & 0b00111111]);
		} else if (leftover_count == 2) {
			out.put(BASE64_ENCODE[(bytes >> 18) & 0b00111111]);
			out.put(BASE64_ENCODE[(bytes >> 12) & 0b00111111]);
			out.put(BASE64_ENCODE[(bytes >> 6) & 0b00111111]);
		}
		// Pad the remaining bytes with '='
		for (int i = 3; i > leftover_count; i--) {
			out.put('=');
		}
	}

	if (!out) {
		std::cerr << "Error: output stream encountered an error!\n";
		return false;
	}

	// No errors encountered
	return true;
}

// This follows the modern RFC 4648 format which does not expect line breaks in the input
bool base64_decode(std::istream& in, std::ostream& out) {
	// Initial file length measurement
	in.seekg(0, std::ios::end);
	if (in.tellg() % 4 != 0) {
		std::cerr << "Error: Base64 string length is not a multiple of 4!\n";
		return false;
	}
	in.seekg(0);

	// Choosing 4kb as it is divisble by and nice
	const std::size_t CHUNK_SIZE = 4 * 1024;
	std::vector<char> buffer(CHUNK_SIZE);
	// Extra array to carry the leftover bits in case of incomplete reads
	char leftover[3];
	std::size_t leftover_count = 0;

	bool padding_seen = false;

	std::vector<char> out_buffer;
	out_buffer.reserve(CHUNK_SIZE * 3 / 4 + 8);

	while (true) {
		// Copy the leftover bytes from last operation into the buffer first
		for (std::size_t i = 0; i < leftover_count; i++) buffer[i] = leftover[i];

		// Read the next chunk of bytes
		in.read(buffer.data() + leftover_count, CHUNK_SIZE - leftover_count);
		std::streamsize bytes_read = in.gcount();

		// Error checks
		if (in.bad()) {
			std::cerr << "Error: input stream encountered an I/O error!\n";
			return false;
		}
		if (in.fail() && !in.eof()) {
			std::cerr << "Error: input stream encountered an unknown error!\n";
			return false;
		}


		if (bytes_read == 0) break; // Reached EOF
		auto total_bytes = bytes_read + leftover_count;
		leftover_count = 0;


		// Process the current buffer
		// If the end of buffer is reached without 4 whole chars to process then add those chars to the
		// leftover buffer to be processed in the next iteration or at the end
		std::size_t i = 0;
		std::vector<char> out_buffer;
		out_buffer.reserve(CHUNK_SIZE * 3 / 4 + 8);
		while (i + 4 <= total_bytes) {
			char a = buffer[i++];
			char b = buffer[i++];
			char c = buffer[i++];
			char d = buffer[i++];

			// Check for padding '='
			if (a == '=' or b == '=') {
				std::cerr << "Error: '=' found at unexpected postion!\n";
				return false;
			}
			int padding = 0;
			if (c == '=') {
				padding++;
				if (d != '=') {
					std::cerr << "Error: '=' found at unexpected postion!\n";
					return false;
				}
			}
			if (d == '=') padding++;
			// Check if the '=' are correct
			if (padding_seen && padding == 0) {
				std::cerr << "Error: data found after padding!\n";
				return false;
			}
			if (padding > 0) {
				padding_seen = true;
			}

			std::uint32_t bytes = 0;
			auto decode_char = [&](char x) -> std::optional<std::uint32_t> {
				if (x == '=') {
					return 0;
				}

				auto it = BASE64_DECODE.find(x);
				if (it == BASE64_DECODE.end()) {
					return std::nullopt;
				}

				return it->second;
				};

			auto v0 = decode_char(a);
			auto v1 = decode_char(b);
			auto v2 = decode_char(c);
			auto v3 = decode_char(d);

			if (!v0 || !v1 || !v2 || !v3) {
				std::cerr << "Error: invalid Base64 character!\n";
				return false;
			}

			bytes =
				(*v0 << 18) |
				(*v1 << 12) |
				(*v2 << 6) |
				(*v3);

			out_buffer.push_back((bytes >> 16) & 0b11111111);

			// Deal with the padding bytes
			if (padding < 2) {
				out_buffer.push_back((bytes >> 8) & 0xFF);
			}
			if (padding < 1) {
				out_buffer.push_back(bytes & 0xFF);
			}
		}

		while (i < total_bytes) {
			leftover[leftover_count++] = buffer[i++];
		}

		// Empty the buffer into the stream
		out.write(out_buffer.data(), out_buffer.size());
		out_buffer.clear();
	}

	// Closing error checks
	if (leftover_count != 0) {
		std::cerr << "Error: Base64 string length is not a multiple of 4!\n";
		return false;
	}
	if (!out) {
		std::cerr << "Error: output stream encountered an error!\n";
		return false;
	}

	return true;
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
		std::cout << "\tb64.exe -d SGVsbG8gV29ybGQ= -o output.txt";
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

		// File input -> stream encoder
		if (!input_file.empty()) {
			std::ifstream input(input_file, std::ios::binary);
			if (!input) {
				std::cerr << "Error: could not open file: " << input_file << '\n';
				return 1;
			}

			if (!output_file.empty()) {
				std::ofstream output(output_file, std::ios::trunc);
				if (!output) {
					std::cerr << "Error: could not open output file: " << output_file << '\n';
					return 1;
				}

				if (!base64_encode(input, output)) {
					std::cout << "Aborting, operation failed!!!\n";
					return 1;
				}
			} else {
				if (!base64_encode(input, std::cout)) {
					std::cout << "Aborting, operation failed!!!\n";
					return 1;
				}
				std::cout << '\n';
			}
		}

		// Inline input -> normal encoder
		else {
			std::istringstream input(inline_input);

			if (!output_file.empty()) {
				std::ofstream output(output_file, std::ios::trunc);
				if (!output) {
					std::cerr << "Error: could not open output file: " << output_file << '\n';
					return 1;
				}


				if (!base64_encode(input, output)) {
					std::cout << "Aborting, operation failed!!!\n";
					return 1;
				}
			} else {
				if (!base64_encode(input, std::cout)) {
					std::cout << "Aborting, operation failed!!!\n";
					return 1;
				}
				std::cout << '\n';
			}
		}
	}

	// Handle decode operations
	else {

		// File input -> stream decoder
		if (!input_file.empty()) {
			std::ifstream input(input_file);
			if (!input) {
				std::cerr << "Error: could not open file: "
					<< input_file << '\n';
				return 1;
			}

			if (!output_file.empty()) {
				std::ofstream output(output_file, std::ios::binary);
				if (!output) {
					std::cout << "Error: could not open output file: " << output_file << '\n';
					return 1;
				}

				if (!base64_decode(input, output)) {
					std::cout << "Aborting, operation failed!!!\n";
					return 1;
				}
			} else {
				if (!base64_decode(input, std::cout)) {
					std::cout << "Aborting, operation failed!!!\n";
					return 1;
				}
				std::cout << '\n';
			}
		}

		// Inline input -> normal decoder
		else {
			std::istringstream input(inline_input);

			if (!output_file.empty()) {
				std::ofstream output(output_file, std::ios::binary);
				if (!output) {
					std::cerr << "Error: could not open output file: " << output_file << '\n';
					return 1;
				}

				if (!base64_decode(input, output)) {
					std::cout << "Aborting, operation failed!!!\n";
					return 1;
				}
			} else {
				if (!base64_decode(input, std::cout)) {
					std::cout << "Aborting, operation failed!!!\n";
					return 1;
				}
				std::cout << '\n';
			}
		}
	}

	return 0;
}