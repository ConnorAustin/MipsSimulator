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
    
    std::string ALUOP_to_text() {
        switch(control.ALUOp) {
            case ALUOP::Add: return "Add";
            case ALUOP::Sub: return "Sub";
            case ALUOP::And: return "And";
            case ALUOP::Or:  return "Or";
            case ALUOP::Nor: return "Nor";
            case ALUOP::Slt: return "Slt";
            case ALUOP::Sll: return "Sll";
            case ALUOP::Srl: return "Srl";
            case ALUOP::Bne: return "Sub";
            case ALUOP::Unknown: return "???";
        }
    }
    
    void draw(sf::RenderWindow& window, sf::Font& font) override {
        sf::Text opText;
        opText.setColor(sf::Color(20, 20, 20));
        opText.setFont(font);
        opText.setCharacterSize(12);
        opText.setString(ALUOP_to_text());
        auto text_size = opText.getGlobalBounds();
        opText.setPosition((int)(x + w / 2 - text_size.width / 2), (int)(y + h / 2 - text_size.height));
        window.draw(opText);
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