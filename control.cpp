#include <map>

#include "control.hpp"

Control control;

// Add, Sub, And, Or, Nor, Slt, Sll, Srl
std::map<u32, ALUOP> funct_to_aluop =
{{0x20, ALUOP::Add}, {0x22,  ALUOP::Sub}, {0x24,  ALUOP::And}, 
{0x25,  ALUOP::Or},  {0x27,  ALUOP::Nor}, {0x2A,  ALUOP::Slt},
{0x00,  ALUOP::Sll}, {0x02,  ALUOP::Srl}};

enum State {
    Fetch = 0,
    Decode,
    Jump,
    JumpRegister,
    Exec_R,
    Branch,
    Exec_LWSW,
    Writeback_R,
    Shift,
    SLT,
    Exec_I,
    Writeback_I,
    LW_Read,
    SW,
    Writeback_LW,
    JAL
};

void set_fetch_state() {
    control.name = "Fetch";
    control.state = Fetch;
    
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
    control.state = Decode;
    
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

void set_exec_r_state(u32 funct) {
    control.name = "Execute R-Type";
    control.state = Exec_R;
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 1;
    control.ALUSrcB = 0;
    control.RegWrite = false;
    control.RegDst = 0; // DC
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0; // DC
    control.IRWrite = false;
    
    control.ALUOp = funct_to_aluop[funct];
}

void set_exec_i_state(ALUOP operation) {
    control.name = "Execute I-Type";
    control.state = Exec_I;
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 1;
    control.ALUSrcB = 2;
    control.RegWrite = false;
    control.RegDst = 0; // DC
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0; // DC
    control.IRWrite = false;
    
    control.ALUOp = operation;
}

void set_jump_state() {
    control.name = "Jump";
    control.state = Jump;
    
    control.PCWriteCond = false;
    control.PCWrite = true;
    control.PCSource = 2;
    control.ALUSrcA = 0;
    control.ALUSrcB = 3;
    control.RegWrite = false;
    control.RegDst = 0;
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0; 
    control.IRWrite = false;
    control.ALUOp = ALUOP::Add;
}

void set_jal_state() {
    control.name = "JAL";
    control.state = JAL;
    
    control.PCWriteCond = false;
    control.PCWrite = true;
    control.PCSource = 2;
    control.ALUSrcA = 0;
    control.ALUSrcB = 3;
    control.RegWrite = true;
    control.RegDst = 2;
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 2; 
    control.IRWrite = false;
    control.ALUOp = ALUOP::Add;
}

void set_jr_state() {
    control.name = "Jump Register";
    control.state = JumpRegister;
    
    control.PCWriteCond = false;
    control.PCWrite = true;
    control.PCSource = 3;
    control.ALUSrcA = 0;
    control.ALUSrcB = 0;
    control.RegWrite = false;
    control.RegDst = 0;
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0; 
    control.IRWrite = false;
    control.ALUOp = ALUOP::Add;
}

void set_branch_state(bool is_beq) {
    control.name = "Branch";
    control.state = Branch;
    
    control.PCWriteCond = true;
    control.PCWrite = false;
    control.PCSource = 1;
    control.ALUSrcA = 1;
    control.ALUSrcB = 0;
    control.RegWrite = false;
    control.RegDst = 0;
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0; 
    control.IRWrite = false;
    control.ALUOp = is_beq ? ALUOP::Sub : ALUOP::Bne;
}

void set_exec_lwsw_state() {
    control.name = "Execute LW/SW";
    control.state = Exec_LWSW;
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 1;
    control.ALUSrcB = 2;
    control.RegWrite = false;
    control.RegDst = 0;
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0; 
    control.IRWrite = false;
    control.ALUOp = ALUOP::Add;
}

void set_writeback_r_state() {
    control.name = "Writeback R-Type";
    control.state = Writeback_R;
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 2;
    control.ALUSrcB = 0;
    control.RegWrite = true;
    control.RegDst = 1;
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0; 
    control.IRWrite = false;
    control.ALUOp = ALUOP::Add;
}

void set_writeback_i_state() {
    control.name = "Writeback I-Type";
    control.state = Writeback_I;
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 0;
    control.ALUSrcB = 0;
    control.RegWrite = true;
    control.RegDst = 0;
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0; 
    control.IRWrite = false;
    control.ALUOp = ALUOP::Add;
}

void set_shift_state(bool sll) {
    control.name = "Shift";
    control.state = Shift;
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 2;
    control.ALUSrcB = 0;
    control.RegWrite = false;
    control.RegDst = 0;
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0;
    control.IRWrite = false;
    
    control.ALUOp = sll ? ALUOP::Sll : ALUOP::Srl;
}

void set_slt_state() {
    control.name = "SLT";
    control.state = SLT;
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 1;
    control.ALUSrcB = 0;
    control.RegWrite = false;
    control.RegDst = 0;
    control.IorD = 0;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 0;
    control.IRWrite = false;
    
    control.ALUOp = ALUOP::Slt;
}

void set_lw_read_state() {
    control.name = "LW Read";
    control.state = LW_Read;
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 0;
    control.ALUSrcB = 0;
    control.RegWrite = false;
    control.RegDst = 0;
    control.IorD = 1;
    control.MemRead = true;
    control.MemWrite = false;
    control.RegDataLoc = 0;
    control.IRWrite = false;
    
    control.ALUOp = ALUOP::Add;
}

void set_sw_state() {
    control.name = "SW";
    control.state = SW;
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 0;
    control.ALUSrcB = 0;
    control.RegWrite = false;
    control.RegDst = 0;
    control.IorD = 1;
    control.MemRead = false;
    control.MemWrite = true;
    control.RegDataLoc = 0;
    control.IRWrite = false;
    
    control.ALUOp = ALUOP::Add;
}

void set_writeback_lw_state() {
    control.name = "Writeback LW";
    control.state = Writeback_LW;
    
    control.PCWriteCond = false;
    control.PCWrite = false;
    control.PCSource = 0;
    control.ALUSrcA = 0;
    control.ALUSrcB = 0;
    control.RegWrite = true;
    control.RegDst = 0;
    control.IorD = 1;
    control.MemRead = false;
    control.MemWrite = false;
    control.RegDataLoc = 1;
    control.IRWrite = false;
    
    control.ALUOp = ALUOP::Add;
}

void next_control_state(u32 instruction) {
    u32 op = (instruction & 0xFC000000) >> 26;
    u32 funct = instruction & 0x0000003F;
    
    switch(control.state) {
        case Fetch:
            set_decode_state();
            break;
        
        case Exec_R:
            set_writeback_r_state();
            break;
        
        case Shift:
            set_writeback_r_state();
            break;
            
        case Writeback_R:
            set_fetch_state();
            break;
            
        case Writeback_I:
            set_fetch_state();
            break;
            
        case Jump:
            set_fetch_state();
            break;
        
        case JAL:
            set_fetch_state();
            break;
        
        case Branch:
            set_fetch_state();
            break;
            
        case SLT:
            set_writeback_r_state();
            break;
        
        case JumpRegister:
            set_fetch_state();
            break;
            
        case Exec_I:
            set_writeback_i_state();
            break;
        
        case SW:
            set_fetch_state();
            break;
            
        case LW_Read:
            set_writeback_lw_state();
            break;
        
        case Writeback_LW:
            set_fetch_state();
            break;
            
        case Exec_LWSW:
            // LW
            if(op == 0x23) { 
                set_lw_read_state();
            } 
            // SW
            else {
                set_sw_state();
            }
            break;
            
        case Decode:
            switch(op) {
                case 0x2: // J
                    set_jump_state();
                    break;
                    
                case 0x3: // JAL
                    set_jal_state();
                    break;
                
                case 0x4: // BEQ
                    set_branch_state(true);
                    break;
                    
                case 0x5: // BNE
                    set_branch_state(false);
                    break;
                    
                case 0x23: // LW
                case 0x2B: // SW
                    set_exec_lwsw_state();
                    break;
                
                case 0x8: // AddI
                    set_exec_i_state(ALUOP::Add);
                    break;
                    
                case 0xC: // AndI
                    set_exec_i_state(ALUOP::And);
                    break;
                
                case 0xD: // OrI
                    set_exec_i_state(ALUOP::Or);
                    break;
                    
                case 0x0: // R-Type 
                    switch(funct) {
                        case 0x0: // SLL
                            set_shift_state(true);
                            break;
                        
                        case 0x2: // SRL
                            set_shift_state(false);
                            break;
                            
                        case 0x8: // JR
                            set_jr_state();
                            break;
                            
                        case 0x2A: // SLT
                            set_slt_state();
                            break;
                            
                        default: // Some R-Type thing
                            set_exec_r_state(funct);
                            break;
                    }
                    break;
            }
            break;
    }
}