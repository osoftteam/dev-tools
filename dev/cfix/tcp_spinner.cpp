#include "tcp_spinner.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <thread>
#include <sys/stat.h>


struct spin_cfg
{
    bool          as_server{false};
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
    char frame_start    {4};
    char frame_end      {3};
    uint16_t protocol   {32};
} prc;

bool load_data_file();
void run_server();
void serve_client(int skt);
bool sendall(int s, char *buf, size_t len);

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
        CHECK_CFG_FIELD("type", prop_val);

        if(prop_val == std::string("server"))
        {
            cfg.as_server = true;
        }
        else if(prop_val == std::string("client"))
        {
            cfg.as_server = false;
        }
        else
        {
            std::cout << "ERROR. tcp_spinner 'type' expected as 'client' or 'server'.]" << std::endl;
            return false;
        }

        CHECK_CFG_FIELD("host", cfg.host);
        CHECK_CFG_FIELD("port", prop_val);
        cfg.port = std::stoi(prop_val);
        CHECK_CFG_FIELD("data-file", cfg.data_file);
        CHECK_CFG_FIELD("spin-sleep-msec", prop_val);
        cfg.spin_sleep_msec = std::stoi(prop_val);
    
        if(cfg.as_server){
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
//        std::cout << "cfg time " << st.st_mtime << std::endl;
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
    
                if(cfg.as_server){
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

void dev::start_tcp_spinner(const std::string& cfg_file)
{
    cfg.cfg_file_path = cfg_file;
    if(!read_full_config_file())
    {
        std::cout << "failed to read file [" << cfg_file << "]\n";
        return;
    }
        
    run_server();
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

        check_read_partial_config_file();
    }
};


void serve_client(int skt)
{
    auto session_start = time(nullptr);
    size_t pnum = 0;
    size_t total_sent = 0;
    size_t print_time = 0;
    while(1)
    {
        if(!sendall(skt, (char*)&prc.frame_start, 1))return;
        uint16_t protocol_h = htons(prc.protocol);
        if(!sendall(skt, (char*)&protocol_h, 2))return;
        uint32_t bdata_len_h = ntohl(prc.bdata_len);
        if(!sendall(skt, (char*)&bdata_len_h, 4))return;
        if(!sendall(skt, (char*)prc.bdata, prc.bdata_len))return;
        if(!sendall(skt, (char*)&prc.frame_end, 1))return;
        if(cfg.spin_sleep_msec > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(cfg.spin_sleep_msec));
        }

        total_sent += prc.bdata_len;
        ++pnum;
        
        auto now = time(nullptr);
        auto time_delta = now - session_start;//&& time_delta > 1
        if(print_time != (int)time_delta && time_delta%2 == 0)
        {
            print_time = (int)time_delta;
            auto bpsec = int(total_sent / time_delta);
            std::cout << "#" << pnum << " " << dev::size_human(total_sent) << " " << dev::size_human(bpsec) << "/sec" << std::endl;
        }        
    }
};


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

bool sendall(int s, char *buf, size_t len)
{
    /*
    auto n = send(s, buf, len, 0);
    std::cout << "sent " << n << " bytes " << std::endl;
    auto b = buf;
    b = buf;
    for(int i=0; i < len;++i){
        std::cout << *b;
        ++b;
    }
    std::cout << std::endl;
    
    for(int i=0; i < len;++i){
        printf("%04X ", *b);
        ++b;
    }
    std::cout << std::endl;*/
    
    size_t total = 0; // how many bytes we've sent
    int bytesleft = len; // how many we have left to send
    int n = 0;
    while(total < len)
    {
       n = send(s, buf+total, bytesleft, 0);
       if (n == -1) {
           perror("socket-send");
           return false;
       }
       total += n;
       bytesleft -= n;
    }
    return true;
};
