# MIPS Simulator
A visualization for a MIPS CPU Simulator I built for *Computer Organization*.


It parses a MIPS asm file and then runs it, opcode by opcode.


![alt-text](https://github.com/ConnorAustin/MipsSimulator/raw/master/example.gif "Example gif")


This is the diagram for the actual MIPS architecture I used for reference:


![alt-text](https://github.com/ConnorAustin/MipsSimulator/raw/master/fig.gif "Figure")


## Running
Requires SFML 2.4 lib along with SFML's dependencies to be installed.


Run `$ make` to build the project and then run `./simulator asm.s` to see it run a simple asm file that sorts a couple numbers.


The up and down arrow keys moves the the window into memory.


The right and left arrow keys speed up and slow down the simulation respectively.
