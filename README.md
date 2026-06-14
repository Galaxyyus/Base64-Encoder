# b64 — Base64 Encoder/Decoder

A lightweight command-line tool for encoding and decoding Base64, written in C++. Supports both inline strings and binary files, with optional file output.

---

## Features

- Encode any string or binary file to Base64
- Decode any Base64 string back to its original bytes
- Read input from the command line or a file
- Write output to stdout or a file

---

## Download
Pre-compiled Windows binaries are available on the [Releases page](../../releases).

---

## Building

Requires a C++11-compatible compiler (e.g. `g++` or `clang++`).

```bash
g++ -std=c++11 -o b64 b64.cpp
```

---

## Usage

```
b64 <OPTIONS> <INPUT> [FLAGS...]
```

### Options

| Option | Description |
|--------|-------------|
| `-e`, `--encode` | Encode input to Base64 |
| `-d`, `--decode` | Decode Base64 input to original bytes |

### Flags

| Flag | Description |
|------|-------------|
| `-f`, `--file <path>` | Read input from a file instead of a command-line string |
| `-o`, `--output <path>` | Write output to a file instead of stdout |

---

## Examples

**Encode a string:**
```bash
b64 -e "Hello World"
# Output: SGVsbG8gV29ybGQ=
```

**Decode a Base64 string:**
```bash
b64 -d SGVsbG8gV29ybGQ=
# Output: Hello World
```

**Encode a binary file and save the result:**
```bash
b64 -e -f input.png -o output.txt
```

**Decode from a file and save the result:**
```bash
b64 -d -f encoded.txt -o output.png
```

**Decode and print to stdout:**
```bash
b64 -d -f encoded.txt
```

---

## Notes

- Inline input and `-f` are mutually exclusive; providing both is an error.
- When encoding binary files, always use `-f` to read the raw bytes correctly.
- When decoding to a binary file, use `-o` to write raw bytes — printing binary to stdout may produce garbled output in terminals.
- Padding characters (`=`) are handled automatically on both encode and decode.

---

## Implementation Details

The encoder processes input in 3-byte chunks, converting each to four 6-bit Base64 characters. Padding (`=`) is appended when the input length is not a multiple of 3.

The decoder processes input in 4-character chunks, reconstructing the original bytes and stripping any trailing padding bytes accordingly.

The Base64 alphabet used follows the standard [RFC 4648](https://datatracker.ietf.org/doc/html/rfc4648) specification (`A–Z`, `a–z`, `0–9`, `+`, `/`).
