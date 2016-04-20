#include <sstream>
#include <iomanip>
#include <map>

#include "util.hpp"
#include "unit.hpp"

class Register : public Unit {
public:
    sf::Text reg_text;

    u32 cur_val;
    u32 next_val;
    Register(std::string title) {
        init(title, 80, 30);
        add_input();
        add_output();
        
        reg_text.setCharacterSize(12);
    }
    
    void draw(sf::RenderWindow& window, sf::Font& font) override {
        reg_text.setFont(font);
        reg_text.setColor(sf::Color(20, 20, 20));
        reg_text.setString(std::to_string(cur_val));
        
        std::stringstream s;
        s << std::setfill('0') << std::setw(8) << std::hex << std::uppercase << cur_val;
        reg_text.setString("0x" + s.str());

        auto size = reg_text.getLocalBounds();
        reg_text.setPosition(x + w / 2.0f - size.width / 2.0f, y + h / 2.0f - size.height);
        window.draw(reg_text);
    }
    
    void on_input(int in, u32 val) override { 
        next_val = val;
    }
    
    void cycle() override { 
        cur_val = next_val;
        write(0, cur_val);
    }
};