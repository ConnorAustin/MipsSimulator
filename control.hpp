#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "util.hpp"

enum class ALUOP {
    Add = 0,
    Sub,
    And,
    Or,
    Nor,
    Slt,
    Sll,
    Srl
};

struct Control {
    std::string name;
    
    u32 PCWriteCond;
    u32 PCWrite; // This is after the OR in the diagram
    u32 PCSource;
    ALUOP ALUOp;
    u32 ALUSrcA;
    u32 ALUSrcB;
    u32 RegWrite;
    u32 RegDst;
    u32 IorD;
    u32 MemRead;
    u32 MemWrite;
    u32 RegDataLoc; // Was MemToReg
    u32 IRWrite;
};

void next_control_state(u32 instruction);

void set_fetch_state();

extern Control control;

#endif