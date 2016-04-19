#include "control.hpp"

Control control;

void set_fetch_state() {
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