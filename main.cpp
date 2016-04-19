// Usage: assembler <options> <input_file>
//   By default it takes input from stdin and prints to stdout and the base address is 0
//   --help        Prints this
//   -a <number>   Sets the base address in hex or decimal as <number>
//   -o <file>     Outputs the assembled result into <file>

#include <exception>
#include <iostream>
#include <cstring>
#include <vector>
#include <string>

#include "assembler.hpp"
#include "util.hpp"
#include "args.hpp"
#include "application.hpp"

// Prints the program's help
void print_help() {
    std::cout << "Usage: assembler <options> <input_file>" << std::endl;
    std::cout << "  By default it takes input from stdin and prints to stdout and the base address is 0" << std::endl;
    std::cout << "  --help        Prints this" << std::endl;
    std::cout << "  -a <number>   Sets the base address in hex or decimal as <number>" << std::endl;
    std::cout << "  -o <file>     Outputs the assembled result into <file>" << std::endl;
}

int main(int argc, char* argv[]) {    
    try {
        //Config config = config_from_args(argc, argv);
        
        //assemble(config.in, config.base_address);
        run_application();
    }
    
    // Handle any errors
    catch(ArgsException e) {
        // Check for the harmless print help with no error string exception 
        if(e.error_message == "" && e.print_help == PrintHelp::yes) {
            print_help();
            return 0;
        }
        std::cerr << "Error: " << e.error_message << std::endl;
        if(e.print_help == PrintHelp::yes) {
            print_help();
        }
        return EXIT_FAILURE;
    }
    catch(AsmException e) {
        if(e.line_number == -1) {
            std::cerr << "Error: ";
        } else {
            std::cerr << "Error on line " << e.line_number << ": ";
        }
        std::cerr << e.error_message << std::endl;
        return EXIT_FAILURE;
    }
    catch(std::ios_base::failure) {
        std::cerr << "IO Error" << std::endl;
    }
    catch(std::exception) {
        std::cerr << "An unknown error occurred" << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
