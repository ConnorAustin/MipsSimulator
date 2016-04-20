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
            auto text_height = text.getLocalBounds().height;
            text.setPosition(x + 5, inputs[i].y - text_height);
            window.draw(text);
        }
    }
    
    void on_input(int in, u32 val) override { 
        if(in == *selector) {
            write(0, val);
        }
    }
};