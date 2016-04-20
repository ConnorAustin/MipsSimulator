#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <cstdint>
#include <vector>
#include "control.hpp"
#include "unit.hpp"
#include "util.hpp"

class Simulator {
private: 
    Unit* add(Unit u);
    
public:
    std::vector<Unit*> circuit;
    
    Simulator(std::vector<u32> instructions, u32 base_address);
    
    void cycle();
    
    void load();
    void save();
};

#endif