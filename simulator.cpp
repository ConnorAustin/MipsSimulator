#include <fstream>

#include "simulator.hpp"
#include "memory.hpp"
#include "register.hpp"
#include "mux.hpp"
#include "alu.hpp"
#include "application.hpp"
#include "constant.hpp"
#include "peeler.hpp"
#include "shifter.hpp"
#include "regfile.hpp"
#include "jmp.hpp"
#include "signextend.hpp"
#include <iostream>
#include <bitset>

const std::string save_filename = "save.txt";

Memory memory;
Register pc("PC");
ALU alu;
Shifter immShift(2);
Jmp jmp;
RegFile regfile;

Register memDataReg("Mem Reg");
Register instrReg("I Reg");
Register aReg("A");
Register bReg("B");
Register aluOut("ALU Out Reg");

Mux memMux(2, &control.IorD);
Mux aluMux1(3, &control.ALUSrcA);
Mux aluMux2(4, &control.ALUSrcB);
Mux pcInMux(4, &control.PCSource);
Mux regDstMux(3, &control.RegDst);
Mux regWriteMux(3, &control.RegDataLoc);

Peeler rs(21, 25);
Peeler rt(16, 20);
Peeler rd(11, 15);
Peeler imm(0, 15);
Peeler jmpImm(0, 25);
Peeler shamt(6, 10);

Constant four(4);
Constant thirtyOne(31);

SignExtend signExtend;

std::vector<std::string> code;

Simulator::Simulator(AsmResult asmResult, u32 base_address) {
    for(int i = 0; i < asmResult.instructions.size(); ++i) {
        std::cout << std::bitset<32>(asmResult.instructions[i]) << " " << asmResult.code[i] << std::endl;
    }
    code = asmResult.code;
    
    set_fetch_state();
    
    // Add all the units
    circuit.push_back(&pc);
    circuit.push_back(&memMux);
    circuit.push_back(&memory);
    circuit.push_back(&memDataReg);
    circuit.push_back(&instrReg);
    circuit.push_back(&alu);
    circuit.push_back(&aluMux1);
    circuit.push_back(&aluMux2);
    circuit.push_back(&aluOut);
    circuit.push_back(&pcInMux);
    circuit.push_back(&four);
    circuit.push_back(&rs);
    circuit.push_back(&rt);
    circuit.push_back(&rd);
    circuit.push_back(&imm);
    circuit.push_back(&jmpImm);
    circuit.push_back(&immShift);
    circuit.push_back(&jmp);
    circuit.push_back(&regfile);
    circuit.push_back(&regDstMux);
    circuit.push_back(&regWriteMux);
    circuit.push_back(&aReg);
    circuit.push_back(&bReg);
    circuit.push_back(&thirtyOne);
    circuit.push_back(&shamt);
    circuit.push_back(&signExtend);
    
    // Connect the units
    pc.connect(0, memMux.get_input(0));
    memMux.connect(0, memory.get_input(0));
    memory.connect(0, memDataReg.get_input(0));
    memory.connect(0, instrReg.get_input(0));
    aluMux1.connect(0, alu.get_input(0));
    pc.connect(0, aluMux1.get_input(0));
    aluMux2.connect(0, alu.get_input(1));
    alu.connect(0, aluOut.get_input(0));
    pcInMux.connect(0, pc.get_input(0));
    alu.connect(0, pcInMux.get_input(0));
    four.connect(0, aluMux2.get_input(1));
    instrReg.connect(0, rs.get_input(0));
    instrReg.connect(0, rt.get_input(0));
    instrReg.connect(0, rd.get_input(0));
    instrReg.connect(0, imm.get_input(0));
    instrReg.connect(0, jmpImm.get_input(0));
    aluOut.connect(0, memory.get_input(1));
    signExtend.connect(0, immShift.get_input(0));
    signExtend.connect(0, aluMux2.get_input(2));
    imm.connect(0, signExtend.get_input(0));
    immShift.connect(0, aluMux2.get_input(3));
    pc.connect(0, jmp.get_input(0));
    jmpImm.connect(0, jmp.get_input(1));
    jmp.connect(0, pcInMux.get_input(2));
    aluOut.connect(0, pcInMux.get_input(1));
    rs.connect(0, regfile.get_input(0));
    rt.connect(0, regfile.get_input(1));
    rt.connect(0, regDstMux.get_input(0));
    rd.connect(0, regDstMux.get_input(1));
    regDstMux.connect(0, regfile.get_input(2));
    aluOut.connect(0, regWriteMux.get_input(0));
    memDataReg.connect(0, regWriteMux.get_input(1));
    regWriteMux.connect(0, regfile.get_input(3));
    regfile.connect(0, aReg.get_input(0));
    regfile.connect(1, bReg.get_input(0));
    aReg.connect(0, aluMux1.get_input(1));
    bReg.connect(0, aluMux2.get_input(0));
    bReg.connect(0, memory.get_input(1));
    aluOut.connect(0, memMux.get_input(1));
    thirtyOne.connect(0, regDstMux.get_input(2));
    pc.connect(0, regWriteMux.get_input(2));
    instrReg.connect(0, shamt.get_input(0));
    shamt.connect(0, aluMux1.get_input(2));
    aReg.connect(0, pcInMux.get_input(3));
    
    // Load the memory with the instructions
    memory.load_instructions(asmResult.instructions, base_address);
    
    // Set the PC to the address where the instructions are
    pc.cur_val = base_address;
    pc.next_val = base_address;
    
    // Try to load the circuit's positions
    load();
}

std::string Simulator::get_code() {
    if(pc.cur_val == 0) {
        return "???";
    }
    
    u32 addr = pc.cur_val - 4;
    if(addr % 4 != 0) {
        return "???";
    }
    addr /= 4;
    if(addr >= code.size()) {
        return "???";
    }
    return code[addr];
}

u32* Simulator::register_values() {
    return regfile.registers;
}

u32 Simulator::lw(u32 address) {
    return memory.lw(address);
}

void Simulator::cycle() {
    // Reset the inputs so that we know when an input is written to
    for(Unit* unit : circuit) {
        unit->reset_inputs();
    }
    
    // Tell the unit's that a cycle is starting
    for(Unit* unit : circuit) {
        unit->cycle();
    }
    
    // Prevent the pc's update if the control disallows it
    if(!control.PCWrite) {
        pc.next_val = pc.cur_val;
    }
    
    // Prevent the instruction register's update if the control disallows it
    if(!control.IRWrite) {
        instrReg.next_val = instrReg.cur_val;
    }
    
    // Tell the unit's that a cycle is ending
    for(Unit* unit : circuit) {
        unit->cycle_end();
    }
    
    u32 instruction = instrReg.cur_val;
    next_control_state(instruction);
}

void Simulator::load() {
    try {
        std::ifstream in;
        in.open(save_filename);
        
        int size;
        in >> size;
        size = std::min((int)circuit.size(), size);
        for(int i = 0; i < size; ++i) {
            int x, y;
            in >> x >> y;
            circuit[i]->set_pos(x, y);
        }
    }
    catch(std::exception e) {
        return;
    }
}

void Simulator::save() {
    std::ofstream out;
    out.open(save_filename);
    out << circuit.size() << std::endl;
    for(int i = 0; i < circuit.size(); ++i) {
        out << circuit[i]->x << ' ' << circuit[i]->y << std::endl;
    }
}

