#include "tcp_spinner.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <thread>
#include <sys/stat.h>
#include "ctf-messenger.h"

enum class erunmode
{
    none,
    server,
    client
};

struct spin_cfg
{
    erunmode       rmode{erunmode::none};
    std::string   host;
    size_t        port;
    std::string   data_file;
    size_t        spin_sleep_msec{100};
    std::string   cfg_file_path;
    time_t        cfg_read_time;
} cfg;

struct spin_state{
    char* bdata         {0};
    uint32_t bdata_len  {0};

} prc;

bool load_data_file();
void run_server();
void run_client();
void serve_client(int skt);

#define CHECK_CFG_FIELD(N, V)i = m.find(N);if(i == m.end()){std::cout << "ERROR. tcp_spinner expected [" << N << "]" << std::endl; return false;}else{V = i->second; std::cout << "cfg " << N " = " << i->second << "\n";}

bool read_full_config_file()
{
    std::cout << "reading cfg [" << cfg.cfg_file_path << "]\n";
    auto r = dev::read_config(cfg.cfg_file_path);
    if(r)
    {
        const dev::S2S& m = r.value();
        std::string prop_val = "";
        auto i =  m.begin();

        CHECK_CFG_FIELD("host", cfg.host);
        CHECK_CFG_FIELD("port", prop_val);
        cfg.port = std::stoi(prop_val);
        CHECK_CFG_FIELD("data-file", cfg.data_file);
        CHECK_CFG_FIELD("spin-sleep-msec", prop_val);
        cfg.spin_sleep_msec = std::stoi(prop_val);
    
        if(cfg.rmode == erunmode::server){
            if(!load_data_file()){
                return false;
            }
        }
    }

    cfg.cfg_read_time = time(nullptr);
    
    return true;
}

bool check_read_partial_config_file()
{
    struct stat st;
    if(stat(cfg.cfg_file_path.c_str(), &st) == 0)
    {
        if(st.st_mtime > cfg.cfg_read_time)
        {
            auto r = dev::read_config(cfg.cfg_file_path);
            if(r)
            {
                const dev::S2S& m = r.value();
                std::string prop_val = "";
                auto i =  m.begin();
                CHECK_CFG_FIELD("data-file", cfg.data_file);
                CHECK_CFG_FIELD("spin-sleep-msec", prop_val);
                cfg.spin_sleep_msec = std::stoi(prop_val);
    
                if(cfg.rmode == erunmode::server){
                    if(!load_data_file()){
                        return false;
                    }
                }
                cfg.cfg_read_time = time(nullptr);
                std::cout << "reloaded cfg file [" << cfg.cfg_file_path << "]" << std::endl;
            }
        }
    }

    return true;
}

void dev::start_tcp_spinner(const std::string& _runmode, const std::string& cfg_file)
{
    if(_runmode == std::string("ctf-server"))
    {
        cfg.rmode = erunmode::server;
    }
    else if(_runmode == std::string("ctf-client"))
    {
        cfg.rmode = erunmode::client;
    }
    else
    {
        std::cout << "ERROR. runmode 'type' expected as 'ctf-client' or 'ctf-server'.]" << std::endl;
        return;
    }
    
    cfg.cfg_file_path = cfg_file;
    if(!read_full_config_file())
    {
        std::cout << "failed to read file [" << cfg_file << "]\n";
        return;
    }

    switch(cfg.rmode)
    {
    case erunmode::server:
    {
        run_server();
    }break;
    case erunmode::client:
    {
        run_client();
    }break;    
    }
};

void run_server()
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    auto skt_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (skt_fd < 0) {
        perror("server/socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(skt_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    memset(&address, 0 , sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, cfg.host.c_str(), &(address.sin_addr));
    address.sin_port = htons(cfg.port);

    if (bind(skt_fd, (struct sockaddr*)&address, addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    while(true)
    {
        std::cout << "listen.. " << std::endl;
        check_read_partial_config_file();
        if (listen(skt_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        auto client_skt = accept(skt_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if(client_skt < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }

        static char client_ip[64] = "";
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::cout << "accepted client [" << client_ip << "]" << std::endl;

        serve_client(client_skt);
        std::cout << "finished [" << client_ip << "]" << std::endl;
    }
};


void serve_client(int skt)
{
    dev::ctf_messenger m;
    m.init(skt, cfg.spin_sleep_msec);
    m.spin_packets(prc.bdata, prc.bdata_len);
};

void read_client_packets(int skt)
{
    dev::ctf_messenger m;
    m.init(skt, cfg.spin_sleep_msec);
    m.receive_packets();
}

bool load_data_file()
{
    std::ifstream f(cfg.data_file, std::ifstream::binary);
    if(f)
    {
        if(prc.bdata)delete[] prc.bdata;
        
        f.seekg(0, f.end);
        prc.bdata_len = static_cast<size_t>(f.tellg());
        f.seekg(0, f.beg);

        prc.bdata = new char[prc.bdata_len];
        f.read(prc.bdata, prc.bdata_len);
        f.close();
        std::cout << "loaded data file [" << prc.bdata_len << "] Bytes" << std::endl;
    }
    else
    {
        std::cout << "ERROR. failed to load file [" << cfg.data_file << std::endl;
        perror("load-file");
        return false;
    }

    return true;
};

void run_client()
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    auto skt_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (skt_fd < 0) {
        perror("client/socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(skt_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    memset(&address, 0 , sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, cfg.host.c_str(), &(address.sin_addr));
    address.sin_port = htons(cfg.port);

    if (connect(skt_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("connect");
        return;
    }

    static char client_ip[64] = "";
    inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
    std::cout << "connected [" << client_ip << "]" << std::endl;

    read_client_packets(skt_fd);    
}
