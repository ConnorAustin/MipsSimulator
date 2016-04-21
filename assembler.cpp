#include <functional>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <array>
#include <regex>
#include <map>

#include "assembler.hpp"
#include "util.hpp"

// The max usable address in mips
const long max_address = (((long)1) << 32) - 1;

// Represents a vector of strings
using Strings = std::vector<std::string>;

// Represents a mapping of labels names to addresses
using Labels = std::map<std::string, long>;

// Array of named registers
// Note their indices are their corresponding value in mips
const std::array<const char*, 32> registers = {{
    "$zero",
    "$at",
    "$v0", "$v1",
    "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9",
    "$k0", "$k1",
    "$gp",
    "$sp",
    "$fp",
    "$ra"
}};

// Contains the details needed to assemble a line of MIPS
struct Instruction {
    const char* name;
    int op;
    int funct;
    std::function<u32(const Strings&, const Labels&, const Instruction&, long)> parser;
};

// Returns the mips value for a named register
//      Throws AsmException
u32 get_register(const std::string& reg) {
    auto it = std::find(std::begin(registers), std::end(registers), reg);
    
    // Assert that we found a legitimate register name
    if(it == std::end(registers)) {
        throw AsmException("'" + reg + "' is not a register");
    }
    
    return std::distance(std::begin(registers), it);
}

// Returns the corresponding address of the label string
//      Throws AsmException
long get_label_address(const Labels& labels, const std::string& label) {
    auto it = labels.find(label);
    
    // Assert that we found it
    if(it == std::end(labels)) {
        throw AsmException("The label '" + label + "' is not defined");
    }
    return it->second;
}

// Asserts that the list of instruction arguments is the expected size_t
//      Throws AsmException
void assert_instruction_len(const Strings& words, int len) {
    if(words.size() < len) {
        throw AsmException("Misssing instruction argument(s)");
    }
    if(words.size() > len) {
        throw AsmException("Excessive instruction argument(s)");
    }
}

// Parses add, sub, and, or, nor, slt
//      Throws AsmException
u32 parse_r(const Strings& words, const Labels& labels, const Instruction& ins, long addr) {
    assert_instruction_len(words, 4);
    u32 rs = get_register(words[2]) << 21;
    u32 rt = get_register(words[3]) << 16;
    u32 rd = get_register(words[1]) << 11;
    u32 funct = ins.funct;
    
    return rs | rt | rd | funct;
}

// Parses addi, andi, ori
//      Throws AsmException
u32 parse_i(const Strings& words, const Labels& labels, const Instruction& ins, long addr) {
    assert_instruction_len(words, 4);
    u32 op = ins.op << 26;
    u32 rs = get_register(words[1]) << 16;
    u32 rt = get_register(words[2]) << 21;
    u32 imm;
    
    // Try to parse the immediate value
    int parsed_imm;
    bool was_hex;
    try {
        auto num = parse_number(words[3]);
        parsed_imm = num.number;
        was_hex = num.was_hex;
    } 
    catch(std::invalid_argument) {
        throw AsmException("'" + words[3] + "' is not a number");
    }
    
    // Assert that the parsed decimal is within the valid range of values
    if(!was_hex && (parsed_imm < -(1 << 15) || parsed_imm > (1 << 15) - 1)) {
        throw AsmException("'" + words[3] + "' is too large in magnitude for " + words[0]);
    }
    
    // Assert that the parsed hex is within the valid range of values
    if(was_hex && (parsed_imm > (1 << 16) - 1)) {
        throw AsmException("'" + words[3] + "' is too large in magnitude for " + words[0]);
    }
    
    imm = parsed_imm & 0xFFFF;
    
    return op | rs | rt | imm;
}

// Parses sll, srl
//      Throws AsmException
u32 parse_shift(const Strings& words, const Labels& labels, const Instruction& ins, long addr) {
    assert_instruction_len(words, 4);
    u32 rt = get_register(words[2]) << 16;
    u32 rd = get_register(words[1]) << 11;
    u32 funct = ins.funct;
    u32 shamt;
    
    // Try to parse the shamt
    int parsed_shamt;
    try {
        parsed_shamt = parse_number(words[3]).number;
    }
    catch(std::invalid_argument) {
        throw AsmException("'" + words[3] + "' is not a number");
    }
    
    // Assert that parsed_shamt is within the valid range of values
    if(parsed_shamt > (1 << 5) - 1 || parsed_shamt < 0) {
        throw AsmException("'" + words[3] + "' is not within the valid range of values for " + words[0]);
    }
    
    shamt = parsed_shamt << 6;
    
    return rt | rd | shamt | funct;
}

// Parses beq, bne
//      Throws AsmException
u32 parse_branch(const Strings& words, const Labels& labels, const Instruction& ins, long addr) {
    assert_instruction_len(words, 4);
    u32 op = ins.op << 26;
    u32 rs = get_register(words[1]) << 21;
    u32 rt = get_register(words[2]) << 16;
    
    int label_addr = get_label_address(labels, words[3]);
    int offset = (((label_addr - (addr + 4)) >> 2) & 0xFFFF);
    
    return op | rs | rt | offset;
}

// Parses lw, sw
//      Throws AsmException
u32 parse_lwsw(const Strings& words, const Labels& labels, const Instruction& ins, long addr) {
    assert_instruction_len(words, 4);
    
    u32 op = ins.op << 26;
    
    // Assert that the offset register is wrapped in parentheses
    const std::string& offset_reg = words[3];
    if(offset_reg[0] != '(' || offset_reg[offset_reg.length() - 1] != ')') {
        throw AsmException("Missing parentheses for " + words[0]);
    }
    
    // Try to parse the offset
    int parsed_offset;
    bool was_hex;
    try {
        auto num = parse_number(words[2]);
        parsed_offset = num.number;
        was_hex = num.was_hex;
    } 
    catch(std::invalid_argument) {
        throw AsmException("'" + words[2] + "' is not a number");
    }
    
    // Assert that the parsed decimal is within the valid range of values
    if(!was_hex && (parsed_offset < -(1 << 15) || parsed_offset > (1 << 15))) {
        throw AsmException("'" + words[2] + "' is too large in magnitude for " + words[0]);
    }
    
    // Assert that the parsed hex is within the valid range of values
    if(was_hex && (parsed_offset > (1 << 16) - 1)) {
        throw AsmException("'" + words[2] + "' is too large in magnitude for " + words[0]);
    }
    
    u32 rs = get_register(offset_reg.substr(1, offset_reg.size() - 2)) << 21;
    u32 rt = get_register(words[1]) << 16;
    int offset = parsed_offset & 0xFFFF;
    return op | rs | rt | offset;
}

// Parses jr
//      Throws AsmException
u32 parse_jr(const Strings& words, const Labels& labels, const Instruction& ins, long addr) {
    assert_instruction_len(words, 2);
    u32 op = ins.op << 26;
    u32 rs = get_register(words[1]) << 21;
    u32 funct = ins.funct;
    
    return op | rs | funct;
}

// Parses j, jal
//      Throws AsmException
u32 parse_j(const Strings& words, const Labels& labels, const Instruction& ins, long addr) {
    assert_instruction_len(words, 2);
    
    long label_addr = get_label_address(labels, words[1]);
    
    u32 op = ins.op << 26;
    long jmp_to = (label_addr >> 2) & 0x3FFFFFF;
    return op | jmp_to;
}

// Array of known instructions
std::array<Instruction, 18> instructions = {{
//   Name    Op  Funct Parser          Name     Op  Funct Parser          Name    Op Funct  Parser
    {"add",   0, 0x20, parse_r},      {"sub",    0, 0x22, parse_r},      {"and",   0, 0x24, parse_r}, 
    {"or",    0, 0x25, parse_r},      {"nor",    0, 0x27, parse_r},      {"slt",   0, 0x2A, parse_r}, 
    {"sll",   0,  0x0, parse_shift},  {"srl",    0,  0x2, parse_shift},  {"jr",    0,  0x8, parse_jr}, 
    {"addi", 0x8,  -1, parse_i},      {"andi", 0xC,   -1, parse_i},      {"ori", 0xD,   -1, parse_i},
    {"beq",  0x4,  -1, parse_branch}, {"bne",  0x5,   -1, parse_branch}, {"lw", 0x23,   -1, parse_lwsw},
    {"sw",   0x2B, -1, parse_lwsw},   {"j",    0x2,   -1, parse_j},      {"jal", 0x3,   -1, parse_j}
}};

// Reads the input from the stream and puts all the lines in a vector
//      Throws std::ios_base::failure
Strings read_input(std::istream* in) {
    // Set the stream to throw a std::ios_base::failure if something goes wrong
    in->exceptions(in->badbit);
    
    Strings lines;
    
    std::string line;
    while(!in->eof()) {
        std::getline(*in, line);
        
        // Remove anything past a comment
        std::regex comments("^([^#]*)#*.*");
        std::smatch matches;
        if(std::regex_match(line, matches, comments)) {
            line = matches[1];
        } else {
            line = "";
        }
        
        lines.push_back(line);
    }
    return lines;
}

// Asserts that the address is a code address or not
//      Throws AsmException
void assert_valid_address(long addr) {
    if(addr % 4 != 0) {
        throw AsmException("The base address is not a multiple of four");
    }
    if(addr > max_address) {
        throw AsmException("Out of valid 32-bit address space");
    }
}

// Reads and strips the labels from the lines
//      Throws AsmException
Labels strip_labels(Strings& lines, long base_address) {
    Labels labels;
    
    long cur_address = base_address;
    
    const auto is_valid_label = [](char c) {
        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c < 'z') || c == '_';
    };
    
    for(int i = 0; i < lines.size(); ++i) {
        std::string& line = lines[i];
        
        // If the line is just whitespace, skip over it
        if(trim(line) == "") {
            continue;
        }
        
        // Check for a label
        if(contains(line, ':')) {
            // Bisect the string by the colon, separating the label from any code
            std::pair<std::string, std::string> s = bisect(line, ':');
            
            // Grab the label
            std::string label = s.first;
            
            // Rewrite the line as just the code part without the label
            line = s.second;
            
            // Assert that the label has a valid name
            if(!std::all_of(std::begin(label), std::end(label), is_valid_label)) {
                throw AsmException(i + 1, "'" + label + "' is not a valid label name");
            }
            
            // Assert that the label is not already defined
            if(labels.find(label) != std::end(labels)) {
                throw AsmException(i + 1, "'" + label + "' is already defined");
            }
            
            // Store our label
            labels[label] = cur_address;
            
            // If the rest of the line did have code, increase the cur_address by four to account for it
            if(trim(line) != "") {
                cur_address += 4;
            }
        } else {
            // The line must just be code, increase cur_address by four
            cur_address += 4;
        }
        assert_valid_address(cur_address);
    }
    return labels;
}

// Returns a cleaned up line to use for assembly parsing
std::string clean_line(std::string line) {
    // Trim the line
    line = trim(line);
    
    // Remove commas
    auto remove_begin = std::remove(std::begin(line), std::end(line), ',');
    line.erase(remove_begin, std::end(line));
    
    // Remove extra whitespace
    // Ex:     add    $s0   $s2 $s5
    // becomes
    //         add $s0 $s2 $s5
    std::regex whitespace("\\s+");
    line = std::regex_replace(line, whitespace, " ");
    
    // Adds space to left of parentheses and removes whitespace in partheneses
    // Ex:     lw $s0 30( $t0 )
    // becomes    
    //         lw $s0 30 ($t0)
    std::regex parentheses_cleanup("\\s*\\(\\s*(\\S.*\\S)\\s*\\)");
    return std::regex_replace(line, parentheses_cleanup, " ($1)");
}

// Assembles a single instruction
//      Throws AsmException
u32 assemble_instruction(const Strings& words, const Labels& labels, long addr) {
    // Look for the instruction that has the name of the first word
    auto it = std::find_if(std::begin(instructions), std::end(instructions), [&](const Instruction& i) {
        return words[0] == i.name;
    });
    
    // Assert we found a legitimate instruction name
    if(it == std::end(instructions)) {
        throw AsmException("'" + words[0] + "' is not a valid instruction");
    }
    
    // Pass it off to the designated parser to parse this particular instruction
    return it->parser(words, labels, *it, addr);
}

// Assembles the given lines of code and outputs the result into output
//      Throws AsmException
//      Throws std::ios_base::failure
AsmResult assemble(std::istream* in, long base_address) {
    AsmResult result;
    
    assert_valid_address(base_address);
    
    Strings lines = read_input(in);
    Labels labels = strip_labels(lines, base_address);
    
    long addr = base_address;
    for(int i = 0; i < lines.size(); ++i) {
        // Grab the clean line, skipping empty ones
        std::string line = clean_line(lines[i]);
        if(line == "") {
            continue;
        }
        
        // Split the instruction up into words
        Strings words = split(line, ' ');
        
        try {
            u32 assembled_instruction = assemble_instruction(words, labels, addr);
            result.instructions.push_back(assembled_instruction);
            result.code.push_back(lines[i]);
        }
        catch(AsmException e) {
            // Tag on the line number to the exception
            e.line_number = i + 1;
            
            // Re-throw the exception
            throw e;
        }
        
        addr += 4;
        assert_valid_address(addr);
    }
    return result;
}
