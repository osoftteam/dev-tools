#include <iostream>
#include <string>
#include <string.h>
#include "cfix.h"
#include <unistd.h>
#include <signal.h>
#include <iomanip>
#include "tcp_spinner.h"
#include "tag-generator.h"

extern bool paint_fix_tags;
bool print_all_data = false;

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
    std::cout << "Example: " << n << " -s fix-server" << " -f data/spin.properties\n";
    std::cout << "Example: " << n << " -s gfix-server" << " -f data/spin.properties\n";    
    std::cout << "Example: " << n << " -s fix-client" << " -f data/spin.properties\n";
    std::cout << std::endl;
    std::cout << "-A - asci/plain text only, no color tags, same as COLORIZE_FIX_TAGS1=none\n";
    std::cout << "-P - print data, busy screen\n";
    std::cout << "-s start as..\n";
    std::cout << "    ctf-server - send data wrapped in CTF-packets\n";
    std::cout << "    ctf-client - consume CTF-packets\n";
    std::cout << "    fix-server - send fix messages wrapped in CTF-packets\n";
    std::cout << "    fix-client - consume(parse) fix messages in CTF-packets\n";
    std::cout << "    gfix-server -generate&send fix messages out of template using config 'tag' options\n";
    std::cout << std::endl;
    std::cout << " env: COLORIZE_FIX_TAGS1=14:32:35:38:39:40:59:150:151:44 COLORIZE_FIX_TAGS2..3\n";
    std::cout << " env: COLORIZE_FIX_TAGS1=none  for 'no-color'\n";
}

void SIGPIPE_handler(int ) {
    printf("Caught SIGPIPE\n");
}

int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIGPIPE_handler);
    std::string cfg_file, runmode;
    cfix::init_cfix();
    bool runtest = false;

    
    if(argc > 1)
    {
        int opt;
        while ((opt = getopt(argc, argv, "vhtf:s:AP")) != -1) {
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
                runtest = true;
            }break;
            case 'f':
            {                
                cfg_file = optarg;
            }break;
            case 's':
            {                
                runmode = optarg;
            }break;
            case 'A':
            {
                paint_fix_tags = false;
            }break;
            case 'P':
            {
                print_all_data = true;
            }break;            
            }
        }//while options
        
        if(runtest)
        {
            cfix::run_test();
            std::cout << std::endl;
            return 0;            
        }
        
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
