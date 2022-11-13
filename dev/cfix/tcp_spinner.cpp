#include "tcp_spinner.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <thread>
#include <sys/stat.h>
#include "ctf-messenger.h"
#include "skt-utils.h"
#include "tag-generator.h"
#include "fix-utils.h"

extern bool use_tcp_protocol;
extern std::string selected_udp_client;

enum class erunmode
{
    none,
    server,
    client,
    fix_server,
    gfix_server,
    fix_client,
};

struct spin_cfg
{
    erunmode             rmode{erunmode::none};
    dev::host_port       hp;
    dev::HOST_PORT_ARR   client_host_ports;
    dev::host_port       udp_cl_hp;
    std::string          data_file;
    std::string          fix_template;    
    size_t               spin_sleep_msec{100};
    std::string          cfg_file_path;
    time_t               cfg_read_time;
    dev::T2G             tag_generators;
    size_t               pkt_counter_tag{0};
} cfg;

struct spin_state{
    dev::datalen_t payload_len{0};
    dev::datalen_t wire_len   {0};
    dev::ctf_packet pkt;
} prc;

bool load_data_file(const std::string& name);
void serve_client(int skt);
void serve_udp_client(int skt);
void serve_client_with_fix_generator(int skt);
void serve_udp_client_with_fix_generator(int skt);
void read_client_packets(int skt);
void read_client_fix_packets(int skt);
void read_udp_client_packets(int skt);
void read_udp_client_fix_packets(int skt);

#define CHECK_CFG_FIELD(N, V)i = m.find(N);if(i == m.end()){std::cout << "ERROR. tcp_spinner expected [" << N << "]" << std::endl; return false;}else{V = i->second; std::cout << "cfg " << N " = " << i->second << "\n";}
#define CHECK_OPT_CFG_FIELD(N, V)i = m.find(N);if(i != m.end()){V = i->second; std::cout << "cfg " << N " = " << i->second << "\n";}

bool read_full_config_file()
{
    std::cout << "reading cfg [" << cfg.cfg_file_path << "]\n";
    auto r = dev::read_config(cfg.cfg_file_path);
    if(r)
    {
        auto& cfg_info  = r.value();
        cfg.tag_generators.clear();
        for(const auto& i : cfg_info.tags){
            auto j = dev::tag_generator_factory::produce_generator(i.second);
            if(j){
                cfg.tag_generators.emplace(i.first, std::move(j.value()));
            }
            else if(i.second == "counter")
            {
                cfg.pkt_counter_tag = i.first;
            }
        }

        for(const auto& i : cfg.tag_generators)
        {
            std::visit([](auto&& g){std::cout << "tag-generator: "<< g << "\n";}, i.second);
        }        
        
        const dev::S2S& m = cfg_info.params;
        std::string prop_val = "";
        auto i =  m.begin();

        CHECK_CFG_FIELD("host", cfg.hp.host);
        CHECK_CFG_FIELD("port", prop_val);
        cfg.hp.port = std::stoi(prop_val);
        CHECK_OPT_CFG_FIELD("data-file", cfg.data_file);
        CHECK_OPT_CFG_FIELD("fix-template", cfg.fix_template);
        CHECK_CFG_FIELD("spin-sleep-msec", prop_val);
        cfg.spin_sleep_msec = dev::stoui(prop_val);

        for(int j = 0; j < 10; ++j){
            std::string client_name = std::string("client") + std::to_string(j);
            std::string prefix = client_name + ".";
            dev::host_port hp;

            i = m.find(prefix + "host");
            if(i != m.end()){
                hp.host = i->second;
            }
            i = m.find(prefix + "port");
            if(i != m.end()){
                hp.port = dev::stoui(i->second);
            }            
            if(!hp.host.empty() && hp.port > 0)
            {
                cfg.client_host_ports.push_back(hp);
                std::cout << "cfg client" << j << "(h,p) = (" << hp.host << "," << hp.port << ")\n";
                if(client_name == selected_udp_client)
                {
                    cfg.udp_cl_hp = hp;
                    std::cout << "cfg selected udp client [" << selected_udp_client << "]";
                }
            }
        }

        
        if(cfg.rmode == erunmode::server){
            if(!load_data_file(cfg.data_file))
            {
                std::cout << "ERROR loading data file [" << cfg.data_file << "]\n";
                return false;
            }
        }
        else if(cfg.rmode == erunmode::fix_server ||
                cfg.rmode == erunmode::gfix_server){
            if(!load_data_file(cfg.fix_template))
            {
                std::cout << "ERROR loading data file [" << cfg.fix_template << "]\n";
                return false;
            }            
        }
    }
    else
    {
        std::cout << "ERROR reading cfg [" << cfg.cfg_file_path << "]\n";
    }

    cfg.cfg_read_time = time(nullptr);
    std::cout << "finished reading cfg [" << cfg.cfg_file_path << "]\n";
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
                auto& cfg_info  = r.value();
                const dev::S2S& m = cfg_info.params;                
                std::string prop_val = "";
                auto i =  m.begin();
                CHECK_CFG_FIELD("data-file", cfg.data_file);
                CHECK_CFG_FIELD("spin-sleep-msec", prop_val);
                cfg.spin_sleep_msec = std::stoi(prop_val);
    
                if(cfg.rmode == erunmode::server)
                {
                    if(!load_data_file(cfg.data_file)){
                        return false;
                    }
                }
                else if(cfg.rmode == erunmode::fix_server ||
                        cfg.rmode == erunmode::gfix_server)
                {
                    if(!load_data_file(cfg.fix_template)){
                        std::cout << "ERROR loading data file [" << cfg.fix_template << "]\n";
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
    else if(_runmode == std::string("fix-server"))
    {
        cfg.rmode = erunmode::fix_server;
    }
    else if(_runmode == std::string("gfix-server"))
    {
        cfg.rmode = erunmode::gfix_server;
    }    
    else if(_runmode == std::string("fix-client"))
    {
        cfg.rmode = erunmode::fix_client;
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
    case erunmode::fix_server:
    {
        if(use_tcp_protocol){
            dev::run_tcp_server(cfg.hp, serve_client);
        }
        else{
            dev::run_udp_server(cfg.hp, serve_udp_client);
        }
    }break;
    case erunmode::gfix_server:
    {
        if(use_tcp_protocol){
            dev::run_tcp_server(cfg.hp, serve_client_with_fix_generator);
        }
        else{
            dev::run_udp_server(cfg.hp, serve_udp_client_with_fix_generator);
        }
    }break;
    case erunmode::client:
    {
        if(use_tcp_protocol){
            dev::run_tcp_client(cfg.hp, read_client_packets);
        }
        else{
            if(cfg.udp_cl_hp.host.empty() || cfg.udp_cl_hp.port == 0){
                std::cout << "ERROR. UDP client bind host/port not defined";
                return;
            }
            dev::run_udp_client(cfg.udp_cl_hp, read_udp_client_packets);
        }
    }break;
    case erunmode::fix_client:
    {
        if(use_tcp_protocol){
            dev::run_tcp_client(cfg.hp, read_client_fix_packets);
        }
        else{
            if(cfg.udp_cl_hp.host.empty() || cfg.udp_cl_hp.port == 0){
                std::cout << "ERROR. UDP client bind host/port not defined";
                return;
            }            
            dev::run_udp_client(cfg.udp_cl_hp, read_udp_client_fix_packets);
        }
    }break;
    default:break;
    }
};


void serve_client(int skt)
{
    check_read_partial_config_file();
    
    dev::ctf_messenger m;
    m.init(skt, cfg.spin_sleep_msec);
    m.spin_packets(prc.pkt, prc.wire_len);
};


void serve_udp_client(int skt)
{
    check_read_partial_config_file();
    
    dev::ctf_messenger<dev::udp_socket> m;
    m.init(skt, cfg.spin_sleep_msec);
    m.sk().setConn(cfg.client_host_ports);
    m.spin_packets(prc.pkt, prc.wire_len);
};


void serve_client_with_fix_generator(int skt)
{
    check_read_partial_config_file();
    
    std::string_view s(prc.pkt.data, prc.payload_len);
    dev::fixmsg_view fv(s);
    dev::ctf_messenger m;
    m.init(skt, cfg.spin_sleep_msec);
    m.generate_fix_packets(fv, cfg.tag_generators, cfg.pkt_counter_tag);
};

void serve_udp_client_with_fix_generator(int skt)
{
    check_read_partial_config_file();
    
    std::string_view s(prc.pkt.data, prc.payload_len);
    dev::fixmsg_view fv(s);
    dev::ctf_messenger<dev::udp_socket> m;
    m.init(skt, cfg.spin_sleep_msec);
    m.sk().setConn(cfg.client_host_ports);
    m.generate_fix_packets(fv, cfg.tag_generators, cfg.pkt_counter_tag);
};


void read_client_packets(int skt)
{
    check_read_partial_config_file();
    
    dev::ctf_messenger m;
    m.init(skt, cfg.spin_sleep_msec);
    m.receive_packets();
}


void read_client_fix_packets(int skt)
{
    check_read_partial_config_file();

    dev::T2STAT stat;
    for(const auto& i : cfg.tag_generators)
    {
        auto tag = i.first;
        std::visit([&stat, &tag](auto&& g){auto v = g.make_tag_stat();stat[tag] = v;}, i.second);
    }
    
    dev::ctf_messenger m;
    m.init(skt, cfg.spin_sleep_msec);
    m.receive_fix_messages(stat, cfg.pkt_counter_tag);
};

void read_udp_client_fix_packets(int skt)
{
    check_read_partial_config_file();
    dev::HOST_PORT_ARR   host_ports;
    host_ports.push_back(cfg.hp);

    
    dev::T2STAT stat;
    for(const auto& i : cfg.tag_generators)
    {
        auto tag = i.first;
        std::visit([&stat, &tag](auto&& g){auto v = g.make_tag_stat();stat[tag] = v;}, i.second);
    }
    
    dev::ctf_messenger<dev::udp_socket> m;
    m.init(skt, cfg.spin_sleep_msec);
    m.sk().setConn(host_ports);
    m.receive_fix_messages(stat, cfg.pkt_counter_tag);
};


void read_udp_client_packets(int skt)
{
    check_read_partial_config_file();

    dev::HOST_PORT_ARR   host_ports;
    host_ports.push_back(cfg.hp);
    
    dev::ctf_messenger<dev::udp_socket> m;
    m.init(skt, cfg.spin_sleep_msec);
    m.sk().setConn(host_ports);
    m.receive_packets();
};

bool load_data_file(const std::string& name)
{
    std::cout << "loading [" << name << "]\n";
    
    std::ifstream f(name, std::ifstream::binary);
    if(f)
    {       
        f.seekg(0, f.end);
        auto bdata_len = static_cast<size_t>(f.tellg());
        f.seekg(0, f.beg);
        if(bdata_len > CTF_MAX_PAYLOAD){
            std::cout << "ERROR. File too big [" << name << "] [" << bdata_len << "]\n";
            return false;
        }

        f.read(prc.pkt.data, bdata_len);
        prc.payload_len = bdata_len;
        prc.wire_len = prc.pkt.setup_packet(bdata_len);
        f.close();
        std::cout << "loaded data file [" << bdata_len << "] Bytes" << std::endl;
    }
    else
    {
        std::cout << "ERROR. failed to load file [" << cfg.data_file << std::endl;
        perror("load-file");
        return false;
    }

    return true;
};
