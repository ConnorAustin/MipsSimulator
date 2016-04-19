#include <algorithm>

#include "unit.hpp"

void Unit::init(std::string title, int w, int h) {
    this->title = title;
    this->x = 20;
    this->y = 20;
    this->w = w;
    this->h = h;
}

void Unit::add_input(u32* val_holder) {
    Input in;
    in.index = inputs.size();
    in.owner = this;
    in.written_to = false;
    in.val_holder = val_holder;
    inputs.push_back(in);
    set_pos(x, y);
}

void Unit::add_input() {
    add_input(nullptr);
}

void Unit::set_pos(int x, int y) {
    this->x = x;
    this->y = y;
    
    for(int i = 0; i < inputs.size(); ++i) {
        inputs[i].x = x;
        inputs[i].y = y + (int)((i + 0.5f) * ((float)h / (float)inputs.size()));
    }
    
    for(int i = 0; i < outputs.size(); ++i) {
        outputs[i].x = x + w;
        outputs[i].y = y + (int)((i + 0.5f) * ((float)h / (float)outputs.size()));
    }
}

void Unit::reset_inputs() { 
    for(Input& in : inputs) {
        in.written_to = false;
    }
}

void Unit::write(int out, u32 val) {
    for(Input* input : outputs[out].busses) {
        if(input->val_holder != nullptr) {
            *input->val_holder = val;
        }
        
        Unit* outputee = input->owner;
        outputee->on_input(input->index, val);
        
        bool all_written_to = true;
        input->written_to = true;
        for(Input& ins : outputee->inputs) {
            if(!ins.written_to) {
                all_written_to = false;
                break;
            }
        }
        
        if(all_written_to) {
            outputee->inputs_ready();
        }
    }
}

void Unit::add_output() {
    Output o;
    outputs.push_back(o);
    set_pos(x, y);
}

Input* Unit::get_input(int in) {
    return &inputs[in];
}

void Unit::connect(int out, Input* input) {
    outputs[out].busses.push_back(input);
}