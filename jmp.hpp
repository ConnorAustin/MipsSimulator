#include <cstring>
#include "util.hpp"
#include "unit.hpp"
#include "control.hpp"

class Jmp : public Unit {
public:
    u32 pc, jmp_imm;
    Jmp() {
        init("Jmp", 20, 20);
        add_input(&pc);
        add_input(&jmp_imm);
        add_output(); // The full jmp immediate
    }
    
    void inputs_ready() override { 
        write(0, (jmp_imm << 2) | (pc & 0xF0000000));
    }
};