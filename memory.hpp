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
    
private:
    std::map<u32, u32> addr_to_mem;
    
    void on_input(int in, u32 val) override { 
        if(in == 0 && control.MemRead) {
            auto it = addr_to_mem.find(address);
            
            if(it == addr_to_mem.end()) {
                write(0, 0);
            } else {
                write(0, it->second);
            }
        }
    }
    
    void load_instructions(u32 base_address, std::vector<u32> instructions) {
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