#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "control.hpp"
#include "unit.hpp"
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

sf::RenderWindow window;
sf::Font font;

// Constants
sf::Color white(230, 230, 230);
sf::Color black(20, 20, 20);
constexpr size_t width  = 1500;
constexpr size_t height = 900;
constexpr size_t cycle_delay = 1000;
constexpr float title_height = 18;
constexpr float io_size = 5.0f;

sf::Vector2i drag_offset;
Unit* drag = nullptr;

u32 selector = 0;

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
Mux pcInMux(3, &control.PCSource);
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

std::vector<Unit*> build_circuit() {
    std::vector<Unit*> result;
    
    result.push_back(&pc);
    result.push_back(&memMux);
    result.push_back(&memory);
    result.push_back(&memDataReg);
    result.push_back(&instrReg);
    result.push_back(&alu);
    result.push_back(&aluMux1);
    result.push_back(&aluMux2);
    result.push_back(&aluOut);
    result.push_back(&pcInMux);
    result.push_back(&four);
    result.push_back(&rs);
    result.push_back(&rt);
    result.push_back(&rd);
    result.push_back(&imm);
    result.push_back(&jmpImm);
    result.push_back(&immShift);
    result.push_back(&jmp);
    result.push_back(&regfile);
    result.push_back(&regDstMux);
    result.push_back(&regWriteMux);
    result.push_back(&aReg);
    result.push_back(&bReg);
    result.push_back(&thirtyOne);
    result.push_back(&shamt);
    
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
    imm.connect(0, immShift.get_input(0));
    imm.connect(0, aluMux2.get_input(2));
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
    return result;
}

void draw_connection(sf::RenderWindow& window, sf::Color& col, int x1, int y1, int x2, int y2) {
    if(x1 < x2) {
        if(std::abs(y1 - y2) < 4) {
            sf::Vertex line[] =
            {
                sf::Vertex(sf::Vector2f(x1, y1), col),
                sf::Vertex(sf::Vector2f(x2, y2), col)
            };
            
            window.draw(line, 2, sf::Lines);
            return;
        }
        
        int half_x = (int)((x1 + io_size) + 0.5f * ((x2 - io_size) - (x1 + io_size)));
        half_x -= half_x % 5;
        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(x1 + io_size, y1), col),
            sf::Vertex(sf::Vector2f(half_x, y1), col),
            
            sf::Vertex(sf::Vector2f(half_x, y1), col),
            sf::Vertex(sf::Vector2f(half_x, y2), col),
            
            sf::Vertex(sf::Vector2f(half_x, y2), col),
            sf::Vertex(sf::Vector2f(x2, y2), col)
        };
        window.draw(line, 6, sf::Lines);
    } else {
        const int jut = 5;
        int jut_out_x = x1 + io_size + jut;
        
        int jut_out_x2 = x2 - io_size - jut;
        float main_y = std::min(y1, y2) - title_height - 50;
        
        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(x1, y1), col),
            sf::Vertex(sf::Vector2f(jut_out_x, y1), col),
            
            sf::Vertex(sf::Vector2f(jut_out_x, y1), col),
            sf::Vertex(sf::Vector2f(jut_out_x, main_y), col),
            
            sf::Vertex(sf::Vector2f(jut_out_x, main_y), col),
            sf::Vertex(sf::Vector2f(jut_out_x2, main_y), col),
            
            sf::Vertex(sf::Vector2f(jut_out_x2, main_y), col),
            sf::Vertex(sf::Vector2f(jut_out_x2, y2), col),
            
            sf::Vertex(sf::Vector2f(jut_out_x2, y2), col),
            sf::Vertex(sf::Vector2f(x2, y2), col)
        };
        window.draw(line, 10, sf::Lines);
    }
}

void draw_unit(Unit* unit) {
    // Draw box 
    sf::RectangleShape box(sf::Vector2f(unit->w, unit->h));
    box.setOutlineThickness(0.7f);
    box.setPosition(unit->x, unit->y);
    box.setFillColor(white);
    box.setOutlineColor(black);
    window.draw(box);
    
    // Draw Title box
    if(unit->title != "") {
        sf::RectangleShape title_box(sf::Vector2f(unit->w, title_height));
        title_box.setPosition(unit->x, unit->y - title_height);
        title_box.setOutlineThickness(0.7f);
        title_box.setFillColor(white);
        title_box.setOutlineColor(black);
        window.draw(title_box);
    }
    
    // Draw title
    sf::Text title;
    title.setColor(black);
    title.setFont(font);
    title.setCharacterSize(10);
    title.setString(unit->title);
    title.setPosition(unit->x + 2, unit->y + 1 - title_height);
    window.draw(title);
    
    // Draw inputs
    for(auto& in : unit->inputs) {
        sf::RectangleShape box(sf::Vector2f(io_size, io_size));
        box.setPosition(in.x - io_size, in.y - io_size / 2.0f);
        box.setFillColor(black);
        window.draw(box);
    }
    
    // Draw outputs
    for(auto& in : unit->outputs) {
        sf::RectangleShape box(sf::Vector2f(io_size, io_size));
        box.setPosition(in.x, in.y - io_size / 2.0f);
        box.setFillColor(black);
        window.draw(box);
    }
    
    // Draw busses
    sf::Color wire_color = black;
    for(auto& out : unit->outputs) {
        for(auto& bus : out.busses) {
            if(bus->owner == drag || unit == drag) {
                wire_color = sf::Color(50, 50, 255);
            } else {
                wire_color = black;
            }
            draw_connection(window, wire_color, out.x, out.y, bus->x, bus->y);
        }
    }
}

void save(const std::vector<Unit*>& circuit) {
    std::ofstream out;
    out.open("save.txt");
    out << circuit.size() << std::endl;
    for(int i = 0; i < circuit.size(); ++i) {
        out << circuit[i]->x << ' ' << circuit[i]->y << std::endl;
    }
}

void load(std::vector<Unit*>& circuit) {
    try {
        std::ifstream in;
        in.open("save.txt");
        
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

void run_application() {
    std::vector<Unit*> circuit = build_circuit();
    load(circuit);
    
    
    set_fetch_state();
    
    
    font.loadFromFile("AndaleMono.ttf");
    
    window.create(sf::VideoMode(width, height), "MIPS my ride");
    window.setFramerateLimit(60);
    
    sf::Clock clk;
    auto prev_cycle_time = clk.getElapsedTime();
    while (window.isOpen()) {
        // Check close event
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        
        // Dragging
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            auto mp = sf::Mouse::getPosition(window);
            if(drag == nullptr) {
                for(Unit* unit : circuit) {
                    if(mp.x >= unit->x && mp.x <= unit->x + unit->w && 
                       mp.y >= unit->y - title_height && mp.y <= unit->y + unit->h) {
                        drag_offset.x = unit->x - mp.x;
                        drag_offset.y = unit->y - mp.y;
                        drag = unit;
                    }
                }
            } else {
                mp += drag_offset;
                mp.x -= mp.x % 2;
                mp.y -= mp.x % 2;
                drag->set_pos(mp.x, mp.y);
            }
        } else {
            drag = nullptr;
        }
        
        // Saving
        static bool save_held = false;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            if(!save_held) {
                save(circuit);
                save_held = true;
            }
        } else {
            save_held = false;
        }
        
        window.clear(sf::Color(200, 200, 200));
        
        for(Unit* unit : circuit) {
            draw_unit(unit);
            unit->draw(window, font);
        }
        
        auto cur_time = clk.getElapsedTime();
        if((cur_time - prev_cycle_time).asMilliseconds() > cycle_delay) {
            prev_cycle_time = cur_time;

            for(Unit* unit : circuit) {
                unit->reset_inputs();
            }
            
            for(Unit* unit : circuit) {
                unit->cycle_start();
            }
            
            // Prevent the pc update if the control disallows it
            if(!control.PCWrite) {
                pc.next_val = pc.cur_val;
            }
            
            // Prevent the instruction register to update if the control disallows it
            if(!control.IRWrite) {
                instrReg.next_val = instrReg.cur_val;
            }
        }
        
        window.display();
    }
}