#include <cstring>
#include "util.hpp"
#include "unit.hpp"
#include "control.hpp"

class Jmp : public Unit {
public:
    u32 pc, jmp_imm;
    Jmp() {
        init("", 24, 24);
        add_input(&pc);
        add_input(&jmp_imm);
        add_output(); // The full jmp immediate
    }
    
    void draw(sf::RenderWindow& window, sf::Font& font) override {
        sf::Text text;
        text.setColor(sf::Color(20, 20, 20));
        text.setFont(font);
        text.setCharacterSize(12);
        text.setString("Jmp");
        auto text_size = text.getGlobalBounds();
        text.setPosition((int)(x + (w - text_size.width) / 2), y);
        window.draw(text);
    }
    
    void inputs_ready() override { 
        write(0, (jmp_imm << 2) | (pc & 0xF0000000));
    }
};