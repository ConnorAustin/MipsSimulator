#include <vector>
#include <map>
#include "util.hpp"
#include "unit.hpp"
#include "control.hpp"

class Memory : public Unit {
public:
    u32 address;
    u32 write_data;
    
    Memory() {
        init("Memory", 40, 40);
        add_input(&address);
        add_input(&write_data);
        add_output(); // Output Data
    }
    
    std::map<u32, u32> addr_to_mem;
    
    u32 lw(u32 address) {
        auto it = addr_to_mem.find(address);
        if(it == addr_to_mem.end()) {
            return 0;
        } else {
            return it->second;
        }
    }
    
    void on_input(int in, u32 val) override { 
        if(in == 0 && control.MemRead) {
            write(0, lw(val));
        }
    }
    
    void load_instructions(std::vector<u32> instructions, u32 base_address) {
        for(int i = 0; i < instructions.size(); ++i) {
            addr_to_mem[base_address + i * 4] = instructions[i];
        }
    }
    
    void inputs_ready() override { 
        if(control.MemWrite) {
            addr_to_mem[address] = write_data;
        }
    }
};