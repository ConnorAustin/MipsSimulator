#include "util.hpp"
#include "unit.hpp"

class SignExtend : public Unit {
public:
    SignExtend() {        
        init("", 88, 20);
        add_input();
        add_output();
    }
    
    void on_input(int in, u32 val) override { 
        u32 sign_bit = val & (1 << 15);
        if(sign_bit != 0) {
            val |= 0xFFFF0000;
        }
        write(0, val);
    }
    
    void draw(sf::RenderWindow& window, sf::Font& font) override {
        sf::Text text;
        text.setColor(sf::Color(20, 20, 20));
        text.setFont(font);
        text.setCharacterSize(12);
        text.setString("Sign Extend ");
        auto text_size = text.getGlobalBounds();
        text.setPosition((int)(x + (w - text_size.width) / 2), y);
        window.draw(text);
    }
};