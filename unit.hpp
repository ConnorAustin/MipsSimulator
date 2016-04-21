#ifndef UNIT_HPP
#define UNIT_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "util.hpp"

class Unit;

struct Input {
    int x, y, index;
    bool written_to;
    Unit* owner;
    u32* val_holder;
};

struct Output {
    int x, y;
    std::vector<Input*> busses;
};

class Unit {
public:
    std::vector<Input> inputs;
    std::vector<Output> outputs;
    
    int x, y, w, h;
    std::string title;
    
    void init(std::string title, int w, int h);
    void add_input();
    void add_input(u32* val_holder);
    void add_output();
    
    Input* get_input(int in);
    
    void connect(int out, Input* input);
    void write(int out, u32 val);
    void set_pos(int x, int y);
    void reset_inputs();
    
    virtual void inputs_ready() { }
    virtual void cycle() { }
    virtual void cycle_end() { }
    virtual void draw(sf::RenderWindow& window, sf::Font& font) { }
    virtual void on_input(int in, u32 val) { }
};

#endif