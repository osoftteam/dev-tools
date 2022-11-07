#include <iostream>
#include <string>
#include <string.h>
#include "pfix.h"
#include <unistd.h>
#include <signal.h>
#include <iomanip>
#include "tcp_spinner.h"
#include "tag-generator.h"

bool print_all_data = false;



static void display_help(const char* n)
{
    std::cout << n << " -s <start-option>\n";
    std::cout << n << " -f <cfg-file> read config file\n";
    std::cout << "Example: " << n << " -s ctf-server" << " -f data/spin.properties\n";
    std::cout << "Example: " << n << " -s ctf-client" << " -f data/spin.properties\n";
    std::cout << "Example: " << n << " -s fix-server" << " -f data/spin.properties\n";
    std::cout << "Example: " << n << " -s gfix-server" << " -f data/spin.properties\n";    
    std::cout << "Example: " << n << " -s fix-client" << " -f data/spin.properties\n";
    std::cout << std::endl;
    std::cout << "-P - print data, busy screen\n";
    std::cout << "-s start as..\n";
    std::cout << "    ctf-server - send data wrapped in CTF-packets\n";
    std::cout << "    ctf-client - consume CTF-packets\n";
    std::cout << "    fix-server - send fix messages wrapped in CTF-packets\n";
    std::cout << "    fix-client - consume(parse) fix messages in CTF-packets\n";
    std::cout << "    gfix-server -generate&send fix messages out of template using config 'tag' options\n";
}

void SIGPIPE_handler(int ) {
    printf("Caught SIGPIPE\n");
}

int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIGPIPE_handler);
    std::string cfg_file, runmode;

    
    if(argc > 1)
    {
        int opt;
        while ((opt = getopt(argc, argv, "hf:s:P")) != -1) {
            switch(opt)
            {
            case 'h':
            {
                display_help(argv[0]);
                return 0;
            }break;
            case 'f':
            {                
                cfg_file = optarg;
            }break;
            case 's':
            {                
                runmode = optarg;
            }break;
            case 'P':
            {
                print_all_data = true;
            }break;            
            }
        }//while options
        
        if(cfg_file.empty()){
            std::cout << "ERROR: expected configuration file\n\n";
            display_help(argv[0]);
            return 0;
        }

        if(runmode.empty()){
            std::cout << "ERROR: expected configuration file\n\n";
            display_help(argv[0]);
            return 0;
        }
        
        dev::start_tcp_spinner(runmode, cfg_file);
    }
    else
    {
        display_help(argv[0]);
    }
    
    return 0;
}
