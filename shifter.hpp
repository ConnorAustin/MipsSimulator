#include "util.hpp"
#include "unit.hpp"

class Shifter : public Unit {
public:
    int shamt;
    
    Shifter(int shamt) {
        this->shamt = shamt;
        
        init("", 50, 20);
        add_input();
        add_output();
    }
    
    void draw(sf::RenderWindow& window, sf::Font& font) override {
        sf::Text text;
        text.setColor(sf::Color(20, 20, 20));
        text.setFont(font);
        text.setCharacterSize(12);
        text.setString("Shift " + std::to_string(shamt));
        auto text_size = text.getGlobalBounds();
        text.setPosition((int)(x + (w - text_size.width) / 2), y);
        window.draw(text);
    }
    
    void on_input(int in, u32 val) override { 
        write(0, val << shamt);
    }
};