#include <iostream>
#include <string>
#include <string.h>
#include "cfix.h"
#include <unistd.h>
#include <signal.h>
#include <iomanip>
#include "tcp_spinner.h"
#include "tag_generator.h"

static bool run_server = false;

void display_verbose_tags_info()
{
    struct tag_info
    {
        size_t tag;
        std::string name;
    };
    
    std::vector<tag_info> tags={{11,"ClOrdID"},
                              {15,"Currency"},
                              {21,"HandlInst"},
                              {22,"IDSource ('4'ISIN)"},
                              {34,"MsgSeqNum"},
                              {38,"OrderQty"},
                              {40,"OrdType"},
                              {48,"SecurityID"},
                              {49,"SenderCompID"},
                              {54,"Side"},
                              {55,"Symbol"},
                              {56,"TargetCompID"},
                              {59,"TimeInForce"},
                              {60,"TransactTime"},
                              {64,"FutSettDate"},
                              {120,"SettlCurrency"}};

    for(const auto& t : tags)
    {
        std::cout << std::setw(5) << std::left << t.tag
                  << std::setw(20) << std::left << t.name << "\n";
    }
};

void display_help(const char* n)
{
    std::cout << n << " -h display help\n";
    std::cout << n << " -v verbose tags list\n";
    std::cout << n << " -t run test\n";
    std::cout << n << " -s <start-option>\n";
    std::cout << n << " -f <cfg-file> read config file\n";
    std::cout << "Example: " << n << " -s ctf-server" << " -f data/spin.properties\n";
    std::cout << "Example: " << n << " -s ctf-client" << " -f data/spin.properties\n";
}

void SIGPIPE_handler(int s) {
    printf("Caught SIGPIPE\n");
}


int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIGPIPE_handler);
    std::string cfg_file, runmode;
    cfix::init_cfix();

    if(argc > 1)
    {
        int opt;
        while ((opt = getopt(argc, argv, "vhtf:s:")) != -1) {
            switch(opt)
            {
            case 'v':
            {
                display_verbose_tags_info();
                return 0;
            }break;
            case 'h':
            {
                display_help(argv[0]);
                return 0;
            }break;
            case 't':
            {
                cfix::run_test();
                std::cout << std::endl;
                return 0;
            }break;
            case 'f':
            {
                cfg_file = optarg;
            }break;
            case 's':
            {                
                runmode = optarg;
            }            
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
        cfix::colorize_stdin();
    }
    
    return 0;
}
