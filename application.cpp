#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>

#include "application.hpp"
#include "unit.hpp"
#include "wire.hpp"

// Constants
const sf::Color white(230, 230, 230);
const sf::Color black(20, 20, 20);
constexpr size_t width  = 1400;
constexpr size_t height = 850;
constexpr size_t cycle_delay = 200;
constexpr float title_height = 18;
constexpr float gui_height = 40;
constexpr int io_size = 5;

sf::RenderWindow window;
sf::Font font;
sf::Texture nextTexture;
sf::Texture autoPlayTexture;
sf::Texture pauseTexture;
sf::Text controlStateText;
sf::Text codeText;
sf::Sprite nextSprite;
sf::Sprite autoPlaySprite;
sf::Sprite pauseSprite;
sf::RectangleShape guiPanel(sf::Vector2f(width, gui_height));
sf::RectangleShape registerPanel(sf::Vector2f(360, 250));
sf::RectangleShape memoryPanel(sf::Vector2f(220, 230));

sf::Vector2i drag_offset;
Unit* drag = nullptr;
bool autoplay = false;
bool paused = true;
float pulse_interp = 0;
float pulse_speed = 0.001f;
u32 cur_address = 0;

void draw_connection(sf::RenderWindow& window, sf::Color& col, int x1, int y1, int x2, int y2) {
    sf::CircleShape pulse(2);
    pulse.setFillColor(col);
    Wire w;
    
    if(x1 < x2) {
        if(std::abs(y1 - y2) < 4) {    
            w.add({{x1, y1}, {x2, y2}});
        }
        else {
            int half_x = (int)((x1 + io_size) + 0.5f * ((x2 - io_size) - (x1 + io_size)));
            half_x -= half_x % 5;
            
            w.add({
                {x1 + io_size, y1},
                {half_x, y1},
                {half_x, y1},
                {half_x, y2},
                {half_x, y2},
                {x2, y2}
            });
        }
    } else {
        const int jut = 5;
        int jut_out_x = x1 + io_size + jut;
        
        int jut_out_x2 = x2 - io_size - jut;
        int main_y = std::min(y1, y2) - title_height - 50;
        
        w.add({
            {x1, y1},
            {jut_out_x, y1},
            {jut_out_x, y1},
            {jut_out_x, main_y},
            {jut_out_x, main_y},
            {jut_out_x2, main_y},
            {jut_out_x2, main_y},
            {jut_out_x2, y2},
            {jut_out_x2, y2},
            {x2, y2}
        });
        w.draw(window, col);
    }
    if(pulse_interp > 0) {
        auto pos = w.interp(pulse_interp);
        pulse.setPosition(pos.x - pulse.getRadius(), pos.y - pulse.getRadius());
        window.draw(pulse);
    }
    w.draw(window, col);
}

void draw_unit(Unit* unit) {
    // Draw box 
    sf::RectangleShape box(sf::Vector2f(unit->w, unit->h));
    box.setOutlineThickness(0.7f);
    box.setPosition(unit->x, unit->y);
    box.setFillColor(white);
    box.setOutlineColor(black);
    window.draw(box);
    
    // Draw Title box
    if(unit->title != "") {
        sf::RectangleShape title_box(sf::Vector2f(unit->w, title_height));
        title_box.setPosition(unit->x, unit->y - title_height);
        title_box.setOutlineThickness(0.7f);
        title_box.setFillColor(white);
        title_box.setOutlineColor(black);
        window.draw(title_box);
    }
    
    // Draw title
    sf::Text title;
    title.setColor(black);
    title.setFont(font);
    title.setCharacterSize(10);
    title.setString(unit->title);
    title.setPosition(unit->x + 2, unit->y + 1 - title_height);
    window.draw(title);
    
    // Draw inputs
    for(auto& in : unit->inputs) {
        sf::RectangleShape box(sf::Vector2f(io_size, io_size));
        box.setPosition(in.x - io_size, in.y - io_size / 2.0f);
        box.setFillColor(black);
        window.draw(box);
    }
    
    // Draw outputs
    for(auto& in : unit->outputs) {
        sf::RectangleShape box(sf::Vector2f(io_size, io_size));
        box.setPosition(in.x, in.y - io_size / 2.0f);
        box.setFillColor(black);
        window.draw(box);
    }
    
    // Draw busses
    sf::Color wire_color = black;
    if(unit == drag) {
        wire_color = sf::Color(50, 50, 255);
    }
    for(auto& out : unit->outputs) {
        for(auto& bus : out.busses) {
            draw_connection(window, wire_color, out.x, out.y, bus->x, bus->y);
        }
    }
}

void draw_registers(Simulator* sim) {
    const std::string register_names[] = {
        "$zero",
        "$at  ",
        "$v0  ", "$v1  ",
        "$a0  ", "$a1  ", "$a2  ", "$a3  ",
        "$t0  ", "$t1  ", "$t2  ", "$t3  ", "$t4  ", "$t5  ", "$t6  ", "$t7  ",
        "$s0  ", "$s1  ", "$s2  ", "$s3  ", "$s4  ", "$s5  ", "$s6  ", "$s7  ",
        "$t8  ", "$t9  ",
        "$k0  ", "$k1  ",
        "$gp  ",
        "$sp  ",
        "$fp  ",
        "$ra  "
    };
    
    sf::Text regText;
    regText.setFont(font);
    regText.setCharacterSize(13);
    regText.setColor(white);
    auto registers = sim->register_values();
    for(int i = 0; i < 16; ++i) {
        regText.setPosition(registerPanel.getPosition().x + 20, registerPanel.getPosition().y + i * 15 + 4);
        regText.setString(register_names[i] + " = "+ to_hex(registers[i]));
        window.draw(regText);
    }
    
    for(int i = 16; i < 32; ++i) {
        regText.setPosition(registerPanel.getPosition().x + 200, registerPanel.getPosition().y + (i - 16) * 15 + 4);
        regText.setString(register_names[i] + " = "+ to_hex(registers[i]));
        window.draw(regText);
    }
}

void draw_memory(Simulator* sim) {
    u32 start_addr = cur_address;
    
    sf::Text memText;
    memText.setFont(font);
    memText.setCharacterSize(13);
    memText.setColor(white);
    for(int i = 0; i < 18; ++i) {
        memText.setPosition(memoryPanel.getPosition().x + 20, memoryPanel.getPosition().y + i * 12 + 4);
        u32 address = start_addr + i * 4;
        memText.setString(to_hex(address) + ": " + to_hex(sim->lw(address)));
        window.draw(memText);
    }
}

Application::Application(Simulator* sim) {
    simulator = sim;
    
    set_fetch_state();
    
    font.loadFromFile("AndaleMono.ttf");
    
    // Init the control state text
    controlStateText.setColor(black);
    controlStateText.setFont(font);
    controlStateText.setCharacterSize(14);
    controlStateText.setPosition(2, 2);
    
    codeText.setColor(black);
    codeText.setFont(font);
    codeText.setCharacterSize(14);
    codeText.setPosition(2, 20);
    
    // Init the gui panel
    guiPanel.setPosition(0, height - gui_height);
    guiPanel.setFillColor(black);
    
    // Init the register panel
    auto registerPanelSize = registerPanel.getLocalBounds();
    registerPanel.setPosition(width - registerPanelSize.width - 20, height - gui_height - registerPanelSize.height);
    registerPanel.setFillColor(black);
    
    // Init the memory panel
    auto memoryPanelSize = memoryPanel.getLocalBounds();
    memoryPanel.setPosition(width - memoryPanelSize.width - 410, height - gui_height - memoryPanelSize.height);
    memoryPanel.setFillColor(black);
    
    // Init the next button
    nextTexture.loadFromFile("next.png");
    nextSprite.setTexture(nextTexture);
    nextSprite.setScale(0.1f, 0.1f);
    nextSprite.setColor(white);
    nextSprite.setPosition(24, height - gui_height + 8);
    
    // Init the auto play button
    autoPlayTexture.loadFromFile("autoplay.png");
    autoPlaySprite.setTexture(autoPlayTexture);
    autoPlaySprite.setScale(0.1f, 0.1f);
    autoPlaySprite.setColor(white);
    autoPlaySprite.setPosition(60, height - gui_height + 8);
    
    // Init the pause button
    pauseTexture.loadFromFile("pause.png");
    pauseSprite.setTexture(pauseTexture);
    pauseSprite.setScale(0.1f, 0.1f);
    pauseSprite.setColor(white);
    pauseSprite.setPosition(120, height - gui_height + 8);
    
    window.create(sf::VideoMode(width, height), "MIPS my ride");
    window.setFramerateLimit(60);
    
    sf::Clock clk;
    auto prev_cycle_time = clk.getElapsedTime();
    bool mouse_held = false;
    while (window.isOpen()) {
        // Check close event
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        
        // Dragging
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            auto mp = sf::Mouse::getPosition(window);
            
            if(!mouse_held) {
                auto nextBox = nextSprite.getGlobalBounds();
                if(nextBox.contains(mp.x, mp.y)) {
                    autoplay = false;
                    paused = false;
                    if(pulse_interp < 0) {
                        pulse_interp = 0;
                    }
                }
                
                auto autoPlayBox = autoPlaySprite.getGlobalBounds();
                if(autoPlayBox.contains(mp.x, mp.y)) {
                    autoplay = true;
                    paused = false;
                    prev_cycle_time = clk.getElapsedTime();
                }
                
                auto pauseBox = pauseSprite.getGlobalBounds();
                if(pauseBox.contains(mp.x, mp.y)) {
                    paused = true;
                    autoplay = false;
                }
            }
            
            if(drag == nullptr) {
                for(Unit* unit : simulator->circuit) {
                    if(mp.x >= unit->x && mp.x <= unit->x + unit->w && 
                       mp.y >= unit->y - title_height && mp.y <= unit->y + unit->h) {
                        drag_offset.x = unit->x - mp.x;
                        drag_offset.y = unit->y - mp.y;
                        drag = unit;
                    }
                }
            } else {
                mp += drag_offset;
                mp.x -= mp.x % 2;
                mp.y -= mp.x % 2;
                drag->set_pos(mp.x, mp.y);
            }
            mouse_held = true;
        } else {
            drag = nullptr;
            mouse_held = false;
        }
        
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            cur_address -= 4;
        }
        
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            cur_address += 4;
        }
        
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            pulse_speed += 0.001f;
            if(pulse_speed > 1) {
                pulse_speed = 1;
            }
        }
        
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            pulse_speed -= 0.001f;
            if(pulse_speed < 0) {
                pulse_speed = 0;
            }
        }
        
        // Saving
        static bool save_held = false;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            if(!save_held) {
                simulator->save();
                save_held = true;
            }
        } else {
            save_held = false;
        }
        
        window.clear(sf::Color(200, 200, 200));
        
        for(Unit* unit : simulator->circuit) {
            draw_unit(unit);
            unit->draw(window, font);
        }
        
        if(drag != nullptr) {
            draw_unit(drag);
            drag->draw(window, font);
        }
        
        controlStateText.setString(control.name);
        window.draw(guiPanel);
        window.draw(registerPanel);
        window.draw(memoryPanel);
        window.draw(nextSprite);
        window.draw(controlStateText);
        codeText.setString(sim->get_code());
        window.draw(codeText);
        draw_registers(sim);
        draw_memory(sim);
        
        autoPlaySprite.setColor(autoplay ? sf::Color(180, 180, 180) : white);
        window.draw(autoPlaySprite);
        
        pauseSprite.setColor(paused ? sf::Color(180, 180, 180) : white);
        window.draw(pauseSprite);
        
        window.display();
        
        auto cur_time = clk.getElapsedTime();
        if(!paused && pulse_interp >= 0) {
            if(pulse_speed > 0.3f) {
                pulse_interp = 1;
            }
            pulse_interp += pulse_speed;
            prev_cycle_time = cur_time;
        }
        
        if(pulse_interp > 1) {
            simulator->cycle();
            pulse_interp = -1;
        }
        
        if(autoplay && (cur_time - prev_cycle_time).asMilliseconds() > cycle_delay) {
            pulse_interp = 0;
        }
    }
}