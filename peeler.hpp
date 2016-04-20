#include "util.hpp"
#include "unit.hpp"

class Peeler : public Unit {
public:
    int a, b;
    int shift, mask;

    Peeler(int a, int b) {
        this->a = a;
        this->b = b;
        
        for(int i = a; i <= b; ++i) {
            mask |= 1 << i;
        }
        shift = a;
        
        init("", 60, 17);
        
        add_input();
        add_output();
    }
    
    void draw(sf::RenderWindow& window, sf::Font& font) override {
        sf::Text text;
        text.setColor(sf::Color(20, 20, 20));
        text.setFont(font);
        text.setCharacterSize(12);
        text.setString("[" + std::to_string(b) + "-" + std::to_string(a) + "]");
        auto text_size = text.getLocalBounds();
        text.setPosition((int)(x + (w - text_size.width) / 2), y);
        window.draw(text);
    }
    
    void on_input(int in, u32 val) override { 
        val &= mask;
        val >>= shift;
        write(0, val);
    }
};