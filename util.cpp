#include <algorithm>
#include <regex>

#include "util.hpp"

// Returns the string with trailing and leading whitespace removed
std::string trim(const std::string& str) {
    auto begin = str.find_first_not_of(" \t\n\r");
    auto end   = str.find_last_not_of(" \t\n\r");
    if(begin == end) {
        return "";
    }
    return str.substr(begin, end - begin + 1);
}

// Returns a pair of the two string's parts split by the first delimter character found
std::pair<std::string, std::string> bisect(const std::string& str, const char delim) {
    std::vector<std::string> result;
    
    size_t begin = 0;
    size_t end = str.find(delim);
    if(end == std::string::npos) {
        return std::make_pair(str, "");
    }
    return std::make_pair(str.substr(begin, end), str.substr(end + 1, std::string::npos));
}

// Returns a vector of the string's parts split by the delimter character
std::vector<std::string> split(const std::string& str, const char delim) {
    std::vector<std::string> result;
    
    size_t begin = 0;
    size_t end = str.find(delim);
    result.push_back(str.substr(begin, end));
    while(end != std::string::npos) {
        begin = end;
        end = str.find(delim, begin + 1);
        result.push_back(str.substr(begin + 1, end - begin - 1));
    }
    return result;
}

// Returns whether or not a string contains a given character or not
bool contains(const std::string& str, char c) {
    return str.find(c) != std::string::npos;
}

// Returns the parsed number if it is hex or decimal
// and if the number was hex (true) or decimal (false)
//      Throws std::invalid_argument
ParsedNumber parse_number(const std::string& str) {
    // Check if it is decimal
    std::regex decimal("^\\s*([\\+-]?[0-9]+)\\s*$");
    if(std::regex_match(str, decimal)) {
        return ParsedNumber(std::stol(str), false);
    }
    
    //Check if it is hex
    std::regex hex("^\\s*(0x[0-9a-fA-F]+)\\s*$");
    if(std::regex_match(str, hex)) {
        return ParsedNumber(std::stol(str, nullptr, 16), true);
    }
    
    // Not a number
    throw std::invalid_argument("");
}