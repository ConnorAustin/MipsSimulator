#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

// Cleaner name for an 32 bit unsigned integer
using u32 = uint32_t;

// Contains the number that was parsed and whether or not the
// string that was parsed was in hex format or decimal format
struct ParsedNumber {
    long number;
    bool was_hex;
    ParsedNumber(long number, bool was_hex) : number(number), was_hex(was_hex) { }
};

// Returns the string with trailing and leading whitespace removed
std::string trim(const std::string& str);

// Returns a pair of the two string's parts split by the first delimter character found
std::pair<std::string, std::string> bisect(const std::string& str, const char delim);

// Returns a vector of the string's parts split by the delimter character
std::vector<std::string> split(const std::string& str, const char delim);

// Returns whether or not a string contains a given character or not
bool contains (const std::string& str, char c);

// Returns the parsed number if it is hex or decimal
// and if the number was hex (true) or decimal (false)
//      Throws std::invalid_argumentt
ParsedNumber parse_number(const std::string& str);

#endif