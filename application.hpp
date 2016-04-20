#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "simulator.hpp"
class Application {
public:
    Simulator* simulator;
    Application(Simulator* sim);
};

#endif