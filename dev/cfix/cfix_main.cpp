#include <iostream>
#include <string.h>
#include "cfix.h"
#include <unistd.h>
#include "tcp_spinner.h"
#include <signal.h>

static bool run_server = false;

void display_verbose_tags_info()
{
    std::cout << "display_verbose_tags_info" << std::endl;
};

void display_help(const char* n)
{
    std::cout << n << " -h display help" << std::endl;
    std::cout << n << " -v verbose tags list" << std::endl;
    std::cout << n << " -t run test" << std::endl;
    std::cout << n << " -s run server (listening) mode" << std::endl;
    std::cout << n << " -c <cfg-file> read config file" << std::endl;
}

void SIGPIPE_handler(int s) {
    printf("Caught SIGPIPE\n");
}


int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIGPIPE_handler);
    
    cfix::init_cfix();

    if(argc > 1)
    {
        int opt;
        while ((opt = getopt(argc, argv, "vhtc:")) != -1) {
            switch(opt)
            {
            case 'v':
            {
                display_verbose_tags_info();
            }break;
            case 'h':
            {
                display_help(argv[0]);
            }break;
            case 't':
            {
                cfix::run_test();
                std::cout << std::endl;
            }break;
            case 'c':
            {
                std::string file_name = optarg;
                dev::start_tcp_spinner(file_name);
            }break;
            }
        }
    }
    else
    {
        cfix::colorize_stdin();
    }
    
    return 0;
}
