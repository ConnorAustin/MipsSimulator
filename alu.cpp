#include "alu.hpp"

u32 op_add(u32 a, u32 b) {
    return a + b;
}

u32 op_sub(u32 a, u32 b) {
    return a - b;
}

u32 op_and(u32 a, u32 b) {
    return a & b;
}

u32 op_or(u32 a, u32 b) {
    return a | b;
}

u32 op_nor(u32 a, u32 b) {
    return ~(a | b);
}

u32 op_slt(u32 a, u32 b) {
    return a < b ? 1 : 0;
}

u32 op_sll(u32 a, u32 b) {
    return b << a;
}

u32 op_srl(u32 a, u32 b) {
    return b >> a;
}

u32 op_bne(u32 a, u32 b) {
    return a != b ? 0 : 1;
}

std::function<u32(u32, u32)> operations[] = {
    op_add,
    op_sub,
    op_and,
    op_or,
    op_nor,
    op_slt,
    op_sll,
    op_srl,
    op_bne
};

u32 ALU::perform_operation() {
    return operations[(int)control.ALUOp](a, b);
}

