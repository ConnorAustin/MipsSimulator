#include <cstring>
#include "util.hpp"
#include "unit.hpp"
#include "control.hpp"

class RegFile : public Unit {
public:
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
        // Check if both the register reads are ready
        if((in == 0 || in == 1) && inputs[0].written_to && inputs[1].written_to) {
            write(0, registers[reg1]);
            write(1, registers[reg2]);
        }
        
        // If the control doesn't want us to write to the register ...
        if(!control.RegWrite) {
            // ... Just set the write data to what it already is!
            write_data = registers[write_reg];
        }
    }
    
    void cycle() override { 
        registers[write_reg] = write_data;
    }
};