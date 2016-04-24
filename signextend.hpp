#include "util.hpp"
#include "unit.hpp"

class SignExtend : public Unit {
public:
    SignExtend() {        
        init("Sign Extend", 70, 20);
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
};