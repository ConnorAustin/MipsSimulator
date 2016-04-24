#include <sstream>
#include <iomanip>
#include <vector>
#include <map>

#include "util.hpp"
#include "unit.hpp"

class Mux : public Unit {
public:
    u32* selector;

    Mux(int input_size, u32* sel) {
        init("", 20, 20 * input_size);
        
        selector = sel;
        
        for(int i = 0; i < input_size; ++i) {
            add_input();
        }
        
        add_output();
    }
    
    void draw(sf::RenderWindow& window, sf::Font& font) override {
        sf::Text text;
        text.setColor(sf::Color(20, 20, 20));
        text.setFont(font);
        text.setCharacterSize(12);
        for(int i = 0; i < inputs.size(); ++i) {
            text.setString(std::to_string(i));
            auto text_size = text.getGlobalBounds();
            text.setPosition((int)(x + w / 2 - text_size.width / 2), (int)(inputs[i].y - text_size.height));
            window.draw(text);
        }
    }
    
    void on_input(int in, u32 val) override { 
        if(in == *selector) {
            write(0, val);
        }
    }
};