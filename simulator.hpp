#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <cstdint>
#include <vector>
#include "assembler.hpp"
#include "control.hpp"
#include "unit.hpp"
#include "util.hpp"

class Simulator {
private: 
    Unit* add(Unit u);
    u32 base_address;

public:
    std::vector<Unit*> circuit;
    
    Simulator(AsmResult asmResult, u32 base_address);
    
    void cycle();
    
    void load();
    void save();
    
    u32* register_values();
    u32 lw(u32 address);
    std::string get_code();
};

#endif