#include <iostream>
#include <string>
#include <string.h>
#include "pfix.h"
#include <unistd.h>
#include <iomanip>
#include "skt-utils.h"

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


static void display_help(const char* n)
{
    std::cout << n << " -h display help\n";
    std::cout << n << " -v verbose tags list\n";
    std::cout << n << " -t run test\n";
    std::cout << std::endl;
    std::cout << " env: COLORIZE_FIX_TAGS1=14:32:35:38:39:40:59:150:151:44 COLORIZE_FIX_TAGS2..3\n";
    std::cout << " env: COLORIZE_FIX_TAGS1=none  for 'no-color'\n";    
}

int main(int argc, char* argv[])
{    
    cfix::init_cfix();
    bool runtest = false;
    
    if(argc > 1)
    {
        int opt;
        while ((opt = getopt(argc, argv, "vht")) != -1) {
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
            }
        }

        if(runtest)
        {
            cfix::run_test();
            std::cout << std::endl;
            return 0;            
        }
        else
        {
            display_help(argv[0]);
        }
    }
    else
    {
        cfix::colorize_stdin();    
    }

    return 0;
}
