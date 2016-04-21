#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <iostream>
#include <vector>

#include "util.hpp"

// Used for exceptions found in the assembly process
struct AsmException {
    int line_number;
    std::string error_message;
    
    AsmException(std::string error_message) : line_number(-1), error_message(error_message) { }
    AsmException(int line_number, std::string error_message) : line_number(line_number), error_message(error_message) { }
};

struct AsmResult {
    std::vector<u32> instructions;
    std::vector<std::string> code;
};

// Assembles the code given from in and outputs the result to out
//      Throws AsmException
//      Throws std::ios_base::failure
AsmResult assemble(std::istream* in, long base_address);

#endif