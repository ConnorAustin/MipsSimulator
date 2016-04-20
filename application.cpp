#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>

#include "application.hpp"
#include "unit.hpp"

// Constants
const sf::Color white(230, 230, 230);
const sf::Color black(20, 20, 20);
constexpr size_t width  = 1500;
constexpr size_t height = 900;
constexpr size_t cycle_delay = 1000;
constexpr float title_height = 18;
constexpr float io_size = 5.0f;

sf::RenderWindow window;
sf::Font font;
sf::Text controlStateText;

sf::Vector2i drag_offset;
Unit* drag = nullptr;

void draw_connection(sf::RenderWindow& window, sf::Color& col, int x1, int y1, int x2, int y2) {
    if(x1 < x2) {
        if(std::abs(y1 - y2) < 4) {
            sf::Vertex line[] =
            {
                sf::Vertex(sf::Vector2f(x1, y1), col),
                sf::Vertex(sf::Vector2f(x2, y2), col)
            };
            
            window.draw(line, 2, sf::Lines);
            return;
        }
        
        int half_x = (int)((x1 + io_size) + 0.5f * ((x2 - io_size) - (x1 + io_size)));
        half_x -= half_x % 5;
        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(x1 + io_size, y1), col),
            sf::Vertex(sf::Vector2f(half_x, y1), col),
            
            sf::Vertex(sf::Vector2f(half_x, y1), col),
            sf::Vertex(sf::Vector2f(half_x, y2), col),
            
            sf::Vertex(sf::Vector2f(half_x, y2), col),
            sf::Vertex(sf::Vector2f(x2, y2), col)
        };
        window.draw(line, 6, sf::Lines);
    } else {
        const int jut = 5;
        int jut_out_x = x1 + io_size + jut;
        
        int jut_out_x2 = x2 - io_size - jut;
        float main_y = std::min(y1, y2) - title_height - 50;
        
        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(x1, y1), col),
            sf::Vertex(sf::Vector2f(jut_out_x, y1), col),
            
            sf::Vertex(sf::Vector2f(jut_out_x, y1), col),
            sf::Vertex(sf::Vector2f(jut_out_x, main_y), col),
            
            sf::Vertex(sf::Vector2f(jut_out_x, main_y), col),
            sf::Vertex(sf::Vector2f(jut_out_x2, main_y), col),
            
            sf::Vertex(sf::Vector2f(jut_out_x2, main_y), col),
            sf::Vertex(sf::Vector2f(jut_out_x2, y2), col),
            
            sf::Vertex(sf::Vector2f(jut_out_x2, y2), col),
            sf::Vertex(sf::Vector2f(x2, y2), col)
        };
        window.draw(line, 10, sf::Lines);
    }
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

Application::Application(Simulator* sim) {
    simulator = sim;
    
    set_fetch_state();
    
    font.loadFromFile("AndaleMono.ttf");
    controlStateText.setPosition(2, 2);
    controlStateText.setColor(black);
    controlStateText.setFont(font);
    controlStateText.setCharacterSize(12);
    
    window.create(sf::VideoMode(width, height), "MIPS my ride");
    window.setFramerateLimit(60);
    
    sf::Clock clk;
    auto prev_cycle_time = clk.getElapsedTime();
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
        } else {
            drag = nullptr;
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
        window.draw(controlStateText);
        
        window.display();
        
        auto cur_time = clk.getElapsedTime();
        if((cur_time - prev_cycle_time).asMilliseconds() > cycle_delay) {
            prev_cycle_time = cur_time;

            simulator->cycle();
        }
    }
}