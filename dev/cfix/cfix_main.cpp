#include <iostream>
#include <string>
#include <string.h>
#include "pfix.h"
#include <unistd.h>
#include <signal.h>
#include <iomanip>
#include "tcp_spinner.h"
#include "tag-generator.h"
#include "ctf-messenger.h"

namespace dev
{
    bool print_all_data = false;
    bool parse_fix = true;
    bool collect_statistics = true;
};

dev::skt_protocol use_protocol;
std::string selected_udp_client;


static void display_help(const char* n)
{        
    std::cout << n << " -r <run-option>\n";
    std::cout << n << " -f <cfg-file> read config file\n\n";
    std::cout << "Example(tcp): " << n << " -r ctf-server" << " -f data/spin.properties\n";
    std::cout << "Example(tcp): " << n << " -r ctf-client" << " -f data/spin.properties\n";
    std::cout << "Example(tcp): " << n << " -r fix-server" << " -f data/spin.properties\n";
    std::cout << "Example(tcp): " << n << " -r gfix-server" << " -f data/spin.properties\n";
    std::cout << "Example(tcp): " << n << " -r fix-client" << " -f data/spin.properties\n";
    std::cout << std::endl;
    std::cout << "Example(upd): " << n << " -p udp -r ctf-server" << " -f data/spin.properties\n";
    std::cout << "Example(upd): " << n << " -p udp -u client0 -r ctf-client" << " -f data/spin.properties\n";
    std::cout << "Example(upd): " << n << " -p udp -r fix-server" << " -f data/spin.properties\n";
    std::cout << "Example(upd): " << n << " -p udp -r gfix-server" << " -f data/spin.properties\n";        
    std::cout << "Example(upd): " << n << " -p udp -u client0 -r fix-client" << " -f data/spin.properties\n";
    std::cout << std::endl;
    std::cout << "Example(unix): " << n << " -p unix -r ctf-server" << " -f data/spin.properties\n";
    std::cout << "Example(unix): " << n << " -p unix -r ctf-client" << " -f data/spin.properties\n";
    std::cout << "Example(unix): " << n << " -p unix -r fix-server" << " -f data/spin.properties\n";
    std::cout << "Example(unix): " << n << " -p unix -r gfix-server" << " -f data/spin.properties\n";    
    std::cout << "Example(unix): " << n << " -p unix -r fix-client" << " -f data/spin.properties\n";
    std::cout << std::endl;
    std::cout << "";
    std::cout << "-V - verbose, print data, busy screen\n";
    std::cout << "-H <domain> - host info, discover IP addresses\n";
    std::cout << "-r run as..\n";
    std::cout << "    ctf-server - send data wrapped in CTF-packets\n";
    std::cout << "    ctf-client - consume CTF-packets\n";
    std::cout << "    fix-server - send fix messages wrapped in CTF-packets\n";
    std::cout << "    fix-client - consume(parse) fix messages in CTF-packets\n";
    std::cout << "    gfix-server -generate&send fix messages out of template using config 'tag' options\n";
    std::cout << "-p protocol '-p tcp' or '-p udp' '-p unix' (tcp by default)\n";
    std::cout << "-u <upd-client-name> \n";
    std::cout << "-m mode \n";
    std::cout << "    1(default) parse fix - ON, collect statistics - ON\n";
    std::cout << "    2 parse fix - ON, statistics - OFF\n";
    std::cout << "    3 parse fix - OF, statistics - OFF\n";
}


void SIGPIPE_handler(int ) {
    printf("Caught SIGPIPE\n");
}

int main(int argc, char* argv[])
{
/*    dev::datalen_t len = 480;
    auto n = htons(len);
    std::cout << n << " " << ntohs(n) << "\n";

    return 0;
*/  
    
    signal(SIGPIPE, SIGPIPE_handler);
    std::string cfg_file, runmode;
    
    if(argc > 1)
    {
        int opt;
        while ((opt = getopt(argc, argv, "hf:r:VFp:u:m:H:")) != -1) {
            switch(opt)
            {
            case 'H':
            {
                //               std::string host = optarg;
                dev::showip(optarg);
                //std::cout << dev::get_ip() << "\n";
                return 0;
            }break;
            case 'h':
            {
                display_help(argv[0]);
                return 0;
            }break;
            case 'f':
            {                
                cfg_file = optarg;
            }break;
            case 'r':
            {                
                runmode = optarg;
            }break;
            case 'u':
            {                
                selected_udp_client = optarg;
            }break;            
            case 'p':
            {
                if(strcmp(optarg, "tcp") == 0){
                    use_protocol = dev::skt_protocol::tcp;
                }
                else if(strcmp(optarg, "udp") == 0){
                    use_protocol = dev::skt_protocol::udp;
                }
                else if(strcmp(optarg, "unix") == 0){
                    use_protocol = dev::skt_protocol::unix;
                }                
                else{
                    std::cout << "ERROR. Invalid protocol option provided [" << optarg << "]\n";
                    return 0;
                }
            }break;
            case 'V':
            {
                std::cout << "cfg verbose mode\n";
                dev::print_all_data = true;
            }break;
            case 'F':
            {
                std::cout << "cfg fast mode\n";
                dev::collect_statistics = false;
            }break;
            case 'm':
            {
                auto v = dev::stoui(optarg);
                switch(v)
                {
                case 2:
                {
                    dev::parse_fix = true;
                    dev::collect_statistics = false;
                }break;
                case 3:
                {
                    dev::parse_fix = false;
                    dev::collect_statistics = false;
                }break;
                }
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
