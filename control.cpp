#include <map>

#include "control.hpp"

Control control;

// Add, Sub, And, Or, Nor, Slt, Sll, Srl
std::map<u32, ALUOP> funct_to_aluop =
{{0x20, ALUOP::Add}, {0x22,  ALUOP::Sub}, {0x24,  ALUOP::And}, 
{0x25,  ALUOP::Or},  {0x27,  ALUOP::Nor}, {0x2A,  ALUOP::Slt},
{0x00,  ALUOP::Sll}, {0x02,  ALUOP::Srl}};

// Addi, Andi, Ori
std::map<u32, ALUOP> op_to_aluop = 
{{0x08,  ALUOP::Add}, {0x0C,  ALUOP::And}, {0x0D,  ALUOP::Or}};

void set_fetch_state() {
    control.name = "Fetch";
    
    control.PCWriteCond = false;
    control.PCWrite = true;
    control.PCSource = 0;
    control.ALUOp = ALUOP::Add;
    control.ALUSrcA = 0;
    control.ALUSrcB = 1;
    control.RegWrite = false;
    control.RegDst = 0; // DC
    control.IorD = 0;
    control.MemRead = true;
    control.MemWrite = false;
    control.RegDataLoc = 0; // DC
    control.IRWrite = true;
}

void set_decode_state() {
    control.name = "Decode";
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUOp = ALUOP::Add;
    control.ALUSrcA = 0;
    control.ALUSrcB = 3;
    control.RegWrite = false;
    control.RegDst = 0; // DC
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0; // DC
    control.IRWrite = false;
}

void set_r_type_execute(u32 funct) {
    control.name = "R-Type Execute";
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 0;
    control.ALUSrcB = 3;
    control.RegWrite = false;
    control.RegDst = 0; // DC
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0; // DC
    control.IRWrite = false;
    
    control.ALUOp = funct_to_aluop[funct];
}

void next_control_state(u32 instruction) {
    u32 op = instruction & 0xFC000000;
    u32 funct = instruction & 0x0000003F;
    
    if(control.name == "Fetch") {
        set_decode_state();
    } 
    else if(control.name == "Decode") {
        // R-Type Decode
        if(op == 0) {            
            set_r_type_execute(funct);
        }
    }
}