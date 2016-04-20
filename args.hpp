#include <string>
#include <iostream>
#include <fstream>
#include "util.hpp"

// Holds the config made from the arguments passed in
class Config {
public:
    std::istream* in;
    long base_address;
};

// The streams that in points to if we are using file IO
// You should never need to access this for yourself, use config.in instead
static std::ifstream input_file;

// Used in ArgsException
// Tells whether after printing the ArgsException's error
// we should print the help description or not
enum class PrintHelp {
    yes, no
};

struct ArgsException {
    std::string error_message;
    PrintHelp print_help;
    
    ArgsException(PrintHelp help) : print_help(help) { }
    ArgsException(PrintHelp help, std::string error_message) : error_message(error_message), print_help(help) { }
};

// Reads the arguments from main and returns a config determined by the arguments
//      Throws ArgsException
Config config_from_args(int argc, char* argv[]) {
    // Set default config
    Config config;
    config.in = &std::cin;
    config.base_address = 0;
    
    // Search for any help arguments
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "--help") == 0) {
            throw ArgsException(PrintHelp::yes, "");
        }
    }
    
    // Search for arguments
    for(int arg_index = 1; arg_index < argc; ++arg_index) {
        char* arg = argv[arg_index];

        // Check to see if it is an option flag
        if(arg[0] == '-') {
            if(strcmp(arg, "-a") == 0) {
                // Assert that there is a next argument which is the base address number
                if(arg_index == argc - 1) {
                    throw ArgsException(PrintHelp::yes, "Must specify number after -a");
                }
                
                try {
                    config.base_address = parse_number(argv[arg_index + 1]).number;
                }
                catch(std::invalid_argument) {
                    throw ArgsException(PrintHelp::yes, std::string("'") + argv[arg_index + 1] + "' is not a valid hex or decimal number");
                }
                // Increase the arg index by 1 to skip over the base address number argument
                ++arg_index; 
            }
            else {
                // It must be a bad option
                throw ArgsException(PrintHelp::yes, std::string() + "Unknown option: " + arg);
            }
        }
        // It must be the input file argument
        else {
            // Assert that the input stream hasn't already been redirected
            if(config.in != &std::cin) {
                throw ArgsException(PrintHelp::yes, "You can only specify one input file");
            }
            
            input_file.open(arg);
            
            // Assert that we opened the input file stream
            if(!input_file.is_open()) {
                throw ArgsException(PrintHelp::no, std::string() + "Could not open " + arg + " to read");
            }
            config.in = &input_file;
        }
    }
    return config;
}