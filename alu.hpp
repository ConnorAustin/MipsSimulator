#include <functional>
#include <vector>

#include "unit.hpp"
#include "control.hpp"

class ALU : public Unit {
public:
    u32 a;
    u32 b;
    bool zero;
    
    ALU() {
        init("ALU", 80, 30);
        add_input(&a); // A
        add_input(&b); // B
        add_output(); // ALU Out
    }
    
    void draw(sf::RenderWindow& window, sf::Font& font) override {
        
    }
    
    u32 perform_operation();
    
    void inputs_ready() override { 
        u32 val = perform_operation();
        zero = val == 0;
        
        if(zero && control.PCWriteCond) {
            control.PCWrite = true;
        }
        
        write(0, val);
    }
};