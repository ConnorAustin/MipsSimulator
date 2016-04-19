#include "unit.hpp"

class Constant : public Unit { 
private:
    u32 val;
public:
    Constant(u32 val) {
        this->val = val;
        init("", 15, 15);
        add_output(); // Constant output
    }
    
    void draw(sf::RenderWindow& window, sf::Font& font) override {
        sf::Text text;
        text.setColor(sf::Color(20, 20, 20));
        text.setFont(font);
        text.setCharacterSize(12);
        text.setString(std::to_string(val));
        auto text_size = text.getLocalBounds();
        text.setPosition((int)(x + (w - text_size.width) / 2), y);
        window.draw(text);
    }
    
    void cycle_start() override {
        write(0, val);
    }
};