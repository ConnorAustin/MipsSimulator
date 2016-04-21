#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>

struct WirePoint {
    int x, y;
    WirePoint(int x, int y) : x(x), y(y) { }
};

struct Wire {
    std::vector<WirePoint> points;
    
    void add(std::vector<WirePoint> points) {
        this->points = points;
    }
    
    void draw(sf::RenderWindow& window, sf::Color& col) {
        std::vector<sf::Vertex> vertices;
        for(int i = 0; i < points.size(); ++i) {
            vertices.push_back(sf::Vertex(sf::Vector2f(points[i].x, points[i].y), col));
        }
        
        window.draw(&vertices[0], vertices.size(), sf::Lines);
    }
    
    int length(int x1, int y1, int x2, int y2) {
        int diff_x = x1 - x2;
        int diff_y = y1 - y2;
        return (int)sqrtf(diff_x * diff_x + diff_y * diff_y);
    }
    
    WirePoint interpolate(int x1, int y1, int x2, int y2, float interp) {
        return WirePoint(x1 + (x2 - x1) * interp, y1 + (y2 - y1) * interp);
    }
    
    WirePoint interp(float interp) {
        int tot_len = 0;
        for(int i = 1; i < points.size(); i += 2) {
            tot_len += length(points[i].x, points[i].y, points[i - 1].x, points[i - 1].y);
        }
        
        int desired_len = (int)(tot_len * interp);
        
        int cur_len = 0;
        for(int i = 1; i < points.size(); i += 2) {
            int l = length(points[i].x, points[i].y, points[i - 1].x, points[i - 1].y);
            
            if(cur_len + l > desired_len) {
                return interpolate(points[i - 1].x, points[i - 1].y, points[i].x, points[i].y, (float)(desired_len - cur_len) / (float)l);
            }
            cur_len += l;
        }
        return WirePoint(points[points.size() - 1].x, points[points.size() - 1].y);
    }
};