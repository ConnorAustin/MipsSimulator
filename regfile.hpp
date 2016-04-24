#include <cstring>
#include "util.hpp"
#include "unit.hpp"
#include "control.hpp"

class RegFile : public Unit {
public:
    int view_index = 0;
    
    u32 registers[32];
    u32 reg1, reg2, write_reg, write_data;
    
    RegFile() {
        init("Registers", 70, 90);
        add_input(&reg1);
        add_input(&reg2);
        add_input(&write_reg);
        add_input(&write_data);
        add_output(); // Output A
        add_output(); // Output B
        
        write_data = 0;
        write_reg = 0;
        memset(registers, 0, sizeof(u32) * 32); 
    }
    
    void on_input(int in, u32 val) override { 
        // Ensure that the write register is less than 31
        if(write_reg > 31) {
            // If it is, something terrible happened. Set the write register to zero
            write_reg = 0;
        }
        
        // If the control doesn't want us to write to the register ...
        if(!control.RegWrite) {
            // ... Just set the write data to what it already is!
            write_data = registers[write_reg];
        }
    }
    
    void cycle_end() override { 
        registers[write_reg] = write_data;
        registers[0] = 0;
    }
    
    void inputs_ready() override { 
        if(reg1 <= 31) {
            write(0, registers[reg1]);
        } else write(0, 0);
        
        if(reg2 <= 31) {
            write(1, registers[reg2]);
        } else write(1, 0);
    }
};